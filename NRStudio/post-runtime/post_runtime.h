#pragma once
#include <dxgi1_4.h>
#include <nvapi.h>
#include <detours.h>
#include <dxgi1_4.h>
#include <bcrypt.h>
#include <tlhelp32.h>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <string>
namespace NRPost {
using Create=decltype(&NvAPI_D3D12_CreateCuFunction);
using Destroy=decltype(&NvAPI_D3D12_DestroyCuFunction);
using Chain=decltype(&NvAPI_D3D12_LaunchCuKernelChain);
using ChainEx=decltype(&NvAPI_D3D12_LaunchCuKernelChainEx);
static Create create;
static Destroy destroy;
static Chain chain;
static ChainEx chainEx;
static decltype(&NvAPI_D3D12_CreateCuModule) createModule;
static decltype(&NvAPI_D3D12_DestroyCuModule) destroyModule;
struct Entry { ID3D12Device* device; NVDX_ObjectHandle module,function; };
static std::map<NVDX_ObjectHandle,Entry> entries;
static std::shared_mutex mutex;
static std::once_flag once;
static bool enabled=false;
static thread_local bool scope=false;
static HMODULE self;
static const void* blob;
static DWORD blobSize;
static LONG launches=0;
struct Scope { bool old; Scope():old(scope){scope=enabled;} ~Scope(){scope=old;} };
static bool ModelMatches(const wchar_t* path) {
 HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return false;
 BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;
 bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
 if(ok)ok=BCryptCreateHash(alg,&hash,nullptr,0,nullptr,0,0)>=0;
 unsigned char buf[65536],digest[32];DWORD bytes=0;
 while(ok){if(!ReadFile(file,buf,sizeof(buf),&bytes,nullptr)){ok=false;break;}if(!bytes)break;ok=BCryptHashData(hash,buf,bytes,0)>=0;}
 if(ok)ok=BCryptFinishHash(hash,digest,sizeof(digest),0)>=0;
 if(hash)BCryptDestroyHash(hash);if(alg)BCryptCloseAlgorithmProvider(alg,0);CloseHandle(file);
 const unsigned char expected[]={0x82,0x70,0xb3,0x50,0xcd,0x82,0xde,0x5c,0xe8,0x98,0x06,0x87,0x2c,0xdd,0x6b,0x6a,0x92,0x49,0xb8,0x08,0x36,0xb9,0x1b,0xbe,0xb3,0x57,0x34,0x70,0x74,0x4c,0xc2,0x06};
 return ok&&memcmp(digest,expected,32)==0;
}
static NvAPI_Status __cdecl OnCreate(ID3D12Device*d,NVDX_ObjectHandle m,const char*n,NVDX_ObjectHandle*out){
 auto result=create(d,m,n,out);
 if(result!=NVAPI_OK||!scope||!out||!n||strcmp(n,"cc_tinlayout_fused_post_block_swin_1h_32_fp8"))return result;
 Entry entry{d,{},{}};
 auto status=createModule(d,blob,blobSize,&entry.module);
 if(status==NVAPI_OK)status=create(d,entry.module,n,&entry.function);
 if(status!=NVAPI_OK){if(entry.module)destroyModule(d,entry.module);native_log("post-opt fallback: candidate creation status=%d",status);return result;}
 std::unique_lock<std::shared_mutex> guard(mutex);
 if(entries.count(*out)){destroy(d,entry.function);destroyModule(d,entry.module);return result;}
 d->AddRef();entries.emplace(*out,entry);
 native_log("post-opt ready: registers=192 original=%p candidate=%p",*out,entry.function);
 return result;
}
static NvAPI_Status __cdecl OnDestroy(ID3D12Device*d,NVDX_ObjectHandle f){
 std::unique_lock<std::shared_mutex> guard(mutex);
 auto result=destroy(d,f);
 auto it=entries.find(f);
 if(result==NVAPI_OK&&it!=entries.end()){
  auto e=it->second;entries.erase(it);
  destroy(e.device,e.function);destroyModule(e.device,e.module);e.device->Release();
  native_log("post-opt released: launches=%ld",launches);
 }
 return result;
}
template<class T,class F> static NvAPI_Status Launch(F original,ID3D12GraphicsCommandList*c,const T*k,NvU32 n){
 if(!c||!k||!n)return original(c,k,n);
 std::shared_lock<std::shared_mutex> guard(mutex);
 if(n==1){
  auto it=entries.find(k->hFunction);if(it==entries.end())return original(c,k,n);
  T copy=*k;
  if(copy.paramSize!=184||copy.blockDim.x!=32||copy.blockDim.y!=1||copy.blockDim.z!=1)return original(c,k,n);
  copy.hFunction=it->second.function;
  LONG count=InterlockedIncrement(&launches);
  if(count<=2||count%600==0)native_log("post-opt launch=%ld grid=%u,%u,%u",count,copy.gridDim.x,copy.gridDim.y,copy.gridDim.z);
  return original(c,&copy,n);
 }
 // The validated model uses singleton chains. Preserve unknown chain layouts.
 return original(c,k,n);
}
static NvAPI_Status __cdecl OnChain(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS*k,NvU32 n){return Launch(chain,c,k,n);}
static NvAPI_Status __cdecl OnChainEx(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS_EX*k,NvU32 n){return Launch(chainEx,c,k,n);}
static void Install(ID3D12Device*device,const wchar_t* model){
 std::call_once(once,[&]{
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&Install),&self);
  wchar_t path[32768];DWORD len=GetModuleFileNameW(self,path,32768);if(!len||len>=32768)return;
  std::wstring marker(path);marker.resize(marker.find_last_of(L"\\")+1);marker+=L"nr-post-opt.enable";
  if(GetFileAttributesW(marker.c_str())==INVALID_FILE_ATTRIBUTES){native_log("post-opt disabled: no enable marker");return;}
  IDXGIFactory4* factory=nullptr;IDXGIAdapter1* adapter=nullptr;DXGI_ADAPTER_DESC1 desc{};
  HRESULT hr=CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  if(SUCCEEDED(hr))hr=factory->EnumAdapterByLuid(device->GetAdapterLuid(),IID_PPV_ARGS(&adapter));
  if(SUCCEEDED(hr))hr=adapter->GetDesc1(&desc);
  if(adapter)adapter->Release();if(factory)factory->Release();
  if(FAILED(hr)||desc.VendorId!=0x10de||desc.DeviceId!=0x2204||!ModelMatches(model)){native_log("post-opt disabled: adapter/model mismatch");return;}
  auto resource=FindResourceW(self,MAKEINTRESOURCEW(101),RT_RCDATA);if(!resource)return;
  blobSize=SizeofResource(self,resource);blob=LockResource(LoadResource(self,resource));if(!blob||!blobSize)return;
  auto nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!nv)return;
  auto query=reinterpret_cast<void*(__cdecl*)(unsigned)>(GetProcAddress(nv,"nvapi_QueryInterface"));if(!query)return;
  create=reinterpret_cast<Create>(query(0xe2436e22));destroy=reinterpret_cast<Destroy>(query(0xdf295ea6));
  chain=reinterpret_cast<Chain>(query(0x24973538));chainEx=reinterpret_cast<ChainEx>(query(0x846a9bf0));
  createModule=reinterpret_cast<decltype(createModule)>(query(0xad1a677d));destroyModule=reinterpret_cast<decltype(destroyModule)>(query(0x41c65285));
  if(!create||!destroy||!chain||!chainEx||!createModule||!destroyModule)return;
  if(DetourTransactionBegin()!=NO_ERROR)return;
  LONG status=DetourUpdateThread(GetCurrentThread());
  std::vector<HANDLE> threads;
  HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
  if(snapshot==INVALID_HANDLE_VALUE){DetourTransactionAbort();return;}
  THREADENTRY32 item{};item.dwSize=sizeof(item);
  if(Thread32First(snapshot,&item))do{
   if(item.th32OwnerProcessID!=GetCurrentProcessId()||item.th32ThreadID==GetCurrentThreadId())continue;
   HANDLE thread=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_SET_CONTEXT|THREAD_QUERY_INFORMATION,FALSE,item.th32ThreadID);
   if(!thread){status=ERROR_ACCESS_DENIED;break;}
   threads.push_back(thread);
   status=DetourUpdateThread(thread);if(status!=NO_ERROR)break;
  }while(Thread32Next(snapshot,&item));
  CloseHandle(snapshot);
  if(status==NO_ERROR)status=DetourAttach(reinterpret_cast<PVOID*>(&create),OnCreate);
  if(status==NO_ERROR)status=DetourAttach(reinterpret_cast<PVOID*>(&destroy),OnDestroy);
  if(status==NO_ERROR)status=DetourAttach(reinterpret_cast<PVOID*>(&chain),OnChain);
  if(status==NO_ERROR)status=DetourAttach(reinterpret_cast<PVOID*>(&chainEx),OnChainEx);
  if(status!=NO_ERROR){DetourTransactionAbort();for(auto thread:threads)CloseHandle(thread);native_log("post-opt hooks failed=%ld",status);return;}
  status=DetourTransactionCommit();for(auto thread:threads)CloseHandle(thread);if(status!=NO_ERROR){native_log("post-opt hook commit failed=%ld",status);return;}
  // Hooks must not point into an unloaded forwarder. Keep its code for process lifetime.
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Install),&self);
  enabled=true;native_log("post-opt enabled: embedded cubin bytes=%lu",blobSize);
 });
}
}
