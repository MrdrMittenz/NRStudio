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
#include "gpu_profile.h"
#include "gpu_work.h"
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
struct Entry { ID3D12Device* device; NVDX_ObjectHandle module,function,controlModule,controlFunction; unsigned paramBytes,blockY; const char* key; };
static std::map<NVDX_ObjectHandle,Entry> entries;
static std::shared_mutex mutex;
static std::once_flag once;
static bool enabled=false;
static thread_local bool scope=false;
static HMODULE self;
static const void* blob;
static DWORD blobSize;
static const void* preBlob;
static DWORD preBlobSize;
static const void* swinBlob;
static DWORD swinBlobSize;
static const void* previousBlob;
static DWORD previousBlobSize;
static const void* previousSwinBlob;
static DWORD previousSwinBlobSize;
static NVDX_ObjectHandle preparedModule{},preparedFunction{},prepareModule{},prepareFunction{};
static bool preparedEnabled=false;
static LONG launches=0;
static HANDLE originalEvent=nullptr;
static LONG lastMode=-1;
static LONG originalLaunches=0;
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
 if(result!=NVAPI_OK||!scope||!out||!n)return result;
 bool post=!strcmp(n,"cc_tinlayout_fused_post_block_swin_1h_32_fp8");
 bool swin=!strcmp(n,"cc_tinlayout_fused_swin_8h_256_8_chained_fp8");
 bool pre=preBlob&&!strcmp(n,"cc_tinlayout_fused_pre_block_swin_1h_32_1_ds_fp8");
 if(!post&&!swin&&!pre)return result;
 Entry entry{};entry.device=d;entry.paramBytes=pre?264:post?184:88;entry.blockY=(post||pre)?1:8;entry.key=pre?"pre":post?"post":"swin8";
 auto status=createModule(d,pre?preBlob:post?blob:swinBlob,pre?preBlobSize:post?blobSize:swinBlobSize,&entry.module);
 if(status==NVAPI_OK)status=create(d,entry.module,n,&entry.function);
 if(status==NVAPI_OK&&!pre&&(post||previousSwinBlob))status=createModule(d,post?previousBlob:previousSwinBlob,post?previousBlobSize:previousSwinBlobSize,&entry.controlModule);
 if(status==NVAPI_OK&&entry.controlModule)status=create(d,entry.controlModule,n,&entry.controlFunction);
 if(status!=NVAPI_OK){if(entry.controlFunction)destroy(d,entry.controlFunction);if(entry.controlModule)destroyModule(d,entry.controlModule);if(entry.function)destroy(d,entry.function);if(entry.module)destroyModule(d,entry.module);native_log("post-opt fallback: candidate creation status=%d",status);return result;}
 std::unique_lock<std::shared_mutex> guard(mutex);
 if(entries.count(*out)){if(entry.controlFunction)destroy(d,entry.controlFunction);if(entry.controlModule)destroyModule(d,entry.controlModule);destroy(d,entry.function);destroyModule(d,entry.module);return result;}
 d->AddRef();entries.emplace(*out,entry);
 native_log("post-opt ready: kernel=%s registers=%u original=%p candidate=%p baseline=previous-validated",entry.key,(post||pre)?224:240,*out,entry.function);
 return result;
}
static NvAPI_Status __cdecl OnDestroy(ID3D12Device*d,NVDX_ObjectHandle f){
 std::unique_lock<std::shared_mutex> guard(mutex);
 auto result=destroy(d,f);
 auto it=entries.find(f);
 if(result==NVAPI_OK&&it!=entries.end()){
  auto e=it->second;entries.erase(it);
  if(e.controlFunction)destroy(e.device,e.controlFunction);if(e.controlModule)destroyModule(e.device,e.controlModule);
  destroy(e.device,e.function);destroyModule(e.device,e.module);e.device->Release();
  native_log("post-opt released: kernel=%s launches=%ld",e.key,launches);
 }
 return result;
}
template<class T,class F> static NvAPI_Status Launch(F original,ID3D12GraphicsCommandList*c,const T*k,NvU32 n){
 if(!c||!k||!n)return original(c,k,n);
 std::shared_lock<std::shared_mutex> guard(mutex);
 if(n==1){
  auto it=entries.find(k->hFunction);if(it==entries.end())return original(c,k,n);
  T copy=*k;
  if(!copy.pParams||copy.paramSize!=it->second.paramBytes||copy.blockDim.x!=32||copy.blockDim.y!=it->second.blockY||copy.blockDim.z!=1||copy.dynSharedMemBytes)return original(c,k,n);
  // RTX 3090 compares preprocessing only; prepared post and exact Swin8 stay active.
  // Preserve the previous comparison behavior on the unchanged Ada path.
  const bool comparisonTarget=!preBlob||!strcmp(it->second.key,"pre");
  const bool useOriginal=comparisonTarget&&originalEvent&&WaitForSingleObject(originalEvent,0)==WAIT_OBJECT_0;
  if(comparisonTarget){
   LONG previous=InterlockedExchange(&lastMode,useOriginal?1:0);
   if(previous!=(useOriginal?1:0))native_log("post-opt benchmark mode=%s",useOriginal?"original":"optimized");
  }
  if(useOriginal){
   LONG count=InterlockedIncrement(&originalLaunches);
   if(count<=2||count%600==0)native_log("post-opt original launch=%ld",count);
   if(it->second.controlFunction)copy.hFunction=it->second.controlFunction;
   if(!strcmp(it->second.key,"post"))NRGpuWork::Mark(5);
   auto result=original(c,&copy,n);if(!strcmp(it->second.key,"post"))NRGpuWork::Mark(6);return result;
  }
  if(preparedEnabled&&it->second.key==std::string("post")){
   if(auto* slot=NRGpuWork::Prepared(c)){
    memcpy(slot->target,copy.pParams,184);memcpy(&slot->prep.source,slot->target+24,8);
    slot->prep.output=slot->weights->GetGPUVirtualAddress();slot->prep.words=20704/4;
    memcpy(slot->target+184,&slot->prep.output,8);
    NRGpuWork::Mark(5);
    D3D12_RESOURCE_BARRIER barrier{};barrier.Type=D3D12_RESOURCE_BARRIER_TYPE_UAV;barrier.UAV.pResource=slot->weights;
    c->ResourceBarrier(1,&barrier);
    T prep=copy;prep.hFunction=prepareFunction;prep.gridDim={(slot->prep.words+127)/128,1,1};prep.blockDim={128,1,1};prep.dynSharedMemBytes=0;prep.paramSize=sizeof(slot->prep);prep.pParams=&slot->prep;
    auto result=original(c,&prep,1);if(result!=NVAPI_OK)return result;
    c->ResourceBarrier(1,&barrier);
    copy.hFunction=preparedFunction;copy.paramSize=sizeof(slot->target);copy.pParams=slot->target;
    result=original(c,&copy,1);NRGpuWork::Mark(6);InterlockedIncrement(&launches);return result;
   }
  }
  if(!strcmp(it->second.key,"post"))NRGpuWork::Mark(5);
  copy.hFunction=it->second.function;
  LONG count=InterlockedIncrement(&launches);
  if(count<=2||count%600==0)native_log("post-opt launch=%ld kernel=%s grid=%u,%u,%u",count,it->second.key,copy.gridDim.x,copy.gridDim.y,copy.gridDim.z);
  auto result=original(c,&copy,n);if(!strcmp(it->second.key,"post"))NRGpuWork::Mark(6);return result;
 }
 // The validated model uses singleton chains. Preserve unknown chain layouts.
 return original(c,k,n);
}
static NvAPI_Status __cdecl OnChain(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS*k,NvU32 n){return Launch(chain,c,k,n);}
static NvAPI_Status __cdecl OnChainEx(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS_EX*k,NvU32 n){return Launch(chainEx,c,k,n);}
static void Install(ID3D12Device*device,ID3D12GraphicsCommandList*cmd,const wchar_t* model){
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
  if(FAILED(hr)||desc.VendorId!=0x10de||!ModelMatches(model)){native_log("post-opt disabled: adapter/model mismatch");return;}
  auto nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!nv)return;
  auto query=reinterpret_cast<NRGpu::Query>(GetProcAddress(nv,"nvapi_QueryInterface"));if(!query)return;
  unsigned architecture=NRGpu::Architecture(device->GetAdapterLuid(),query);
  auto profile=NRGpu::Select(desc.VendorId,desc.DeviceId,architecture);
  if(profile==NRGpu::Profile::Unsupported){native_log("post-opt disabled: unsupported adapter device=%04x architecture=%x",desc.DeviceId,architecture);return;}
  const bool ada=profile==NRGpu::Profile::Ada;
  std::wstring directory(path);directory.resize(directory.find_last_of(L"\\")+1);
  bool wantPrepared=!ada&&GetFileAttributesW((directory+L"nr-prepared-post.enable").c_str())!=INVALID_FILE_ATTRIBUTES;
  bool wantTiming=GetFileAttributesW((directory+L"nr-gpu-timing.enable").c_str())!=INVALID_FILE_ATTRIBUTES;
  bool tracking=false;

  native_log("post-opt GPU: device=%04x architecture=%x profile=%s",desc.DeviceId,architecture,ada?"Ada sm_89 (hardware validation pending)":"RTX 3090 sm_86");
  auto resource=FindResourceW(self,MAKEINTRESOURCEW(ada?201:101),RT_RCDATA);if(!resource)return;
  blobSize=SizeofResource(self,resource);blob=LockResource(LoadResource(self,resource));if(!blob||!blobSize)return;
  resource=FindResourceW(self,MAKEINTRESOURCEW(ada?202:102),RT_RCDATA);if(!resource)return;
  swinBlobSize=SizeofResource(self,resource);swinBlob=LockResource(LoadResource(self,resource));if(!swinBlob||!swinBlobSize)return;
  resource=FindResourceW(self,MAKEINTRESOURCEW(ada?103:105),RT_RCDATA);if(!resource)return;
  previousBlobSize=SizeofResource(self,resource);previousBlob=LockResource(LoadResource(self,resource));if(!previousBlob||!previousBlobSize)return;
  if(!ada){
   resource=FindResourceW(self,MAKEINTRESOURCEW(104),RT_RCDATA);if(!resource)return;
   previousSwinBlobSize=SizeofResource(self,resource);previousSwinBlob=LockResource(LoadResource(self,resource));if(!previousSwinBlob||!previousSwinBlobSize)return;
  }
  if(!ada){resource=FindResourceW(self,MAKEINTRESOURCEW(108),RT_RCDATA);if(!resource)return;preBlobSize=SizeofResource(self,resource);preBlob=LockResource(LoadResource(self,resource));if(!preBlob)return;}
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
  if(status==NO_ERROR&&(wantPrepared||wantTiming)){
   tracking=NRGpuWork::Attach(device,cmd,wantTiming,directory+L"nr-gpu-timings-"+std::to_wstring(GetCurrentProcessId())+L".csv");
   if(!tracking)native_log("gpu-work unavailable: retaining cumulative kernels");
  }
  if(status!=NO_ERROR){DetourTransactionAbort();for(auto thread:threads)CloseHandle(thread);native_log("post-opt hooks failed=%ld",status);return;}
  status=DetourTransactionCommit();for(auto thread:threads)CloseHandle(thread);if(status!=NO_ERROR){native_log("post-opt hook commit failed=%ld",status);return;}
  // Hooks must not point into an unloaded forwarder. Keep its code for process lifetime.
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Install),&self);
  wchar_t eventName[96];swprintf_s(eventName,L"Local\\NRStudio.PostOriginal.%lu",GetCurrentProcessId());
  originalEvent=CreateEventW(nullptr,TRUE,FALSE,eventName);
  if(!originalEvent)native_log("post-opt benchmark control unavailable error=%lu",GetLastError());
  if(tracking)NRGpuWork::Activate();
  if(tracking&&wantPrepared){
   auto load=[&](int id,const char*name,NVDX_ObjectHandle&m,NVDX_ObjectHandle&fn){
    auto r=FindResourceW(self,MAKEINTRESOURCEW(id),RT_RCDATA);if(!r)return false;
    auto data=LockResource(LoadResource(self,r));auto bytes=SizeofResource(self,r);
    return data&&bytes&&createModule(device,data,bytes,&m)==NVAPI_OK&&create(device,m,name,&fn)==NVAPI_OK;
   };
   preparedEnabled=load(106,"cc_tinlayout_fused_post_block_swin_1h_32_fp8",preparedModule,preparedFunction)&&load(107,"nrPrepareWeights",prepareModule,prepareFunction);
   native_log("prepared-post enabled=%d bounded_slots=%u",preparedEnabled?1:0,NRGpuWork::Capacity);
  }
  enabled=true;native_log("post-opt enabled: cumulative exact pre+post+swin8 update post=%lu swin8=%lu previous=%lu bytes",blobSize,swinBlobSize,previousBlobSize);
 });
}
}
