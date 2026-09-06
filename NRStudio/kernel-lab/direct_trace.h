#pragma once
#include <nvapi.h>
#include <detours.h>
#include <bcrypt.h>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include "chain_timing.h"
#include "direct_specs.h"
namespace NRDirectTrace {
using Create=decltype(&NvAPI_D3D12_CreateCuFunction);
using Chain=decltype(&NvAPI_D3D12_LaunchCuKernelChain);
using ChainEx=decltype(&NvAPI_D3D12_LaunchCuKernelChainEx);
static Create create;
static Chain chain;
static ChainEx chainEx;
static decltype(&NvAPI_D3D12_CreateCuModule) createModule;
static decltype(&NvAPI_D3D12_DestroyCuModule) destroyModule;
static decltype(&NvAPI_D3D12_DestroyCuFunction) destroyFunction;
struct Entry {ID3D12Device* device;NVDX_ObjectHandle module,function,controlModule,controlFunction;const NRDirectSpec* spec;unsigned candidateCalls=0,controlCalls=0;};
static std::map<NVDX_ObjectHandle,Entry> entries;
static std::map<NVDX_ObjectHandle,std::string> names;
static unsigned frame=0;
static bool timingOnly=GetEnvironmentVariableW(L"NRSTUDIO_TIMING_ONLY",nullptr,0)>0;
static std::string mode,keys;
static std::wstring folder;
static BCRYPT_ALG_HANDLE sha=nullptr;
static bool Candidate(){return mode=="candidate"||(mode=="alternate"&&(frame%4==1||frame%4==2));}
static void Load(ID3D12Device*d,const wchar_t*file,const char*name,NVDX_ObjectHandle&m,NVDX_ObjectHandle&f){
 std::ifstream in(folder+L"\\"+file,std::ios::binary);std::vector<char> blob((std::istreambuf_iterator<char>(in)),{});
 if(blob.empty()||createModule(d,blob.data(),static_cast<NvU32>(blob.size()),&m)!=NVAPI_OK||create(d,m,name,&f)!=NVAPI_OK)exit(80);
}
static NvAPI_Status __cdecl OnCreate(ID3D12Device*d,NVDX_ObjectHandle m,const char*n,NVDX_ObjectHandle*out){
 auto result=create(d,m,n,out);if(result!=NVAPI_OK||!n||!out)return result;names[*out]=n;
 for(const auto& spec:NR_DIRECT_SPECS)if(!strcmp(spec.name,n)&&keys.find(","+std::string(spec.key)+",")!=std::string::npos){
  if(entries.count(*out))exit(81);Entry e{};e.device=d;e.spec=&spec;e.controlFunction=*out;
  std::wstring filename;for(const char*c=spec.key;*c;++c)filename+=wchar_t(*c);filename+=L"-direct.cubin";
  Load(d,filename.c_str(),n,e.module,e.function);
  if(!strcmp(spec.key,"post"))Load(d,L"post-word-raw.cubin",n,e.controlModule,e.controlFunction);
  d->AddRef();entries.emplace(*out,e);break;
 }
 return result;
}
template<class T,class F> static NvAPI_Status Launch(F original,ID3D12GraphicsCommandList*c,const T*k,NvU32 n){
 if(!c||!k||!n)return original(c,k,n);
 std::string label=names[k[0].hFunction];std::vector<T> copies(k,k+n);
 for(auto& item:copies){auto it=entries.find(item.hFunction);if(it==entries.end())continue;auto&e=it->second;
  if(n!=1||item.paramSize!=e.spec->bytes||!item.pParams||item.blockDim.x!=32||item.blockDim.y!=e.spec->blockY||item.blockDim.z!=1||item.dynSharedMemBytes)exit(82);
  if(Candidate()){item.hFunction=e.function;++e.candidateCalls;}else{item.hFunction=e.controlFunction;++e.controlCalls;}
 }
 label+=Candidate()?"|candidate":"|control";
 unsigned index=ChainTiming::Begin(c,n,label,label);auto result=original(c,copies.data(),n);ChainTiming::End(c,index);return result;
}
static NvAPI_Status __cdecl OnChain(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS*k,NvU32 n){return Launch(chain,c,k,n);}
static NvAPI_Status __cdecl OnChainEx(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS_EX*k,NvU32 n){return Launch(chainEx,c,k,n);}
static void HashFrame(const void*data,unsigned width,unsigned height,unsigned pitch){
 BCRYPT_HASH_HANDLE hash=nullptr;unsigned char digest[32];
 if(BCryptCreateHash(sha,&hash,nullptr,0,nullptr,0,0)<0)exit(83);
 for(unsigned y=0;y<height;++y)if(BCryptHashData(hash,(PUCHAR)data+size_t(y)*pitch,width*8,0)<0)exit(84);
 if(BCryptFinishHash(hash,digest,32,0)<0)exit(85);BCryptDestroyHash(hash);
 printf("frame_sha256 frame=%u mode=%s hash=",frame,Candidate()?"candidate":"control");
 for(auto value:digest)printf("%02x",value);puts("");
}
static void Finish(){
 if(!ChainTiming::completed)return;ChainTiming::Finish();
 for(auto& pair:entries){auto&e=pair.second;printf("direct: %s candidate_calls=%u control_calls=%u\n",e.spec->key,e.candidateCalls,e.controlCalls);
  destroyFunction(e.device,e.function);destroyModule(e.device,e.module);
  if(e.controlModule){destroyFunction(e.device,e.controlFunction);destroyModule(e.device,e.controlModule);}e.device->Release();
 }
 BCryptCloseAlgorithmProvider(sha,0);
}
static void Install(){
 wchar_t directory[32768];DWORD len=GetEnvironmentVariableW(L"NRSTUDIO_DIRECT_DIR",directory,32768);if(!len||len>=32768)exit(86);folder=directory;
 char text[256];len=GetEnvironmentVariableA("NRSTUDIO_DIRECT_MODE",text,256);if(!len||len>=256)exit(87);mode=text;
 if(mode!="candidate"&&mode!="control"&&mode!="alternate")exit(88);
 len=GetEnvironmentVariableA("NRSTUDIO_DIRECT_KEYS",text,256);if(!len||len>=256)exit(89);keys=","+std::string(text)+",";
 auto nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!nv)exit(90);
 auto q=reinterpret_cast<void*(__cdecl*)(unsigned)>(GetProcAddress(nv,"nvapi_QueryInterface"));if(!q)exit(91);
 create=reinterpret_cast<Create>(q(0xe2436e22));chain=reinterpret_cast<Chain>(q(0x24973538));chainEx=reinterpret_cast<ChainEx>(q(0x846a9bf0));
 createModule=reinterpret_cast<decltype(createModule)>(q(0xad1a677d));destroyModule=reinterpret_cast<decltype(destroyModule)>(q(0x41c65285));destroyFunction=reinterpret_cast<decltype(destroyFunction)>(q(0xdf295ea6));
 if(!create||!chain||!chainEx||!createModule||!destroyModule||!destroyFunction)exit(92);
 if(BCryptOpenAlgorithmProvider(&sha,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)exit(93);
 if(DetourTransactionBegin()!=NO_ERROR||DetourUpdateThread(GetCurrentThread())!=NO_ERROR)exit(94);
 if(DetourAttach(reinterpret_cast<PVOID*>(&create),OnCreate)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&chain),OnChain)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&chainEx),OnChainEx)!=NO_ERROR)exit(95);
 if(DetourTransactionCommit()!=NO_ERROR)exit(96);atexit(Finish);
}
}
