#pragma once
#include <nvapi.h>
#include <detours.h>
#include <vector>
#include <fstream>
#include <cstring>
namespace LaunchBatch {
using Chain=decltype(&NvAPI_D3D12_LaunchCuKernelChain);
using ChainEx=decltype(&NvAPI_D3D12_LaunchCuKernelChainEx);
using Query=HRESULT(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*,REFIID,void**);
static Chain chain;static ChainEx chainEx;static Query queryInterface;
static bool active=false,inside=false,batch=false;
static unsigned frame=0,launches=0,groups=0,limit=256;
static ID3D12GraphicsCommandList* current=nullptr;
static std::ofstream log;
struct Pending{NVAPI_CU_KERNEL_LAUNCH_PARAMS kernel;std::vector<unsigned char> bytes;};
static std::vector<Pending> pending;
static void Flush(){
 if(pending.empty())return;
 std::vector<NVAPI_CU_KERNEL_LAUNCH_PARAMS> kernels;kernels.reserve(pending.size());
 for(auto& p:pending){p.kernel.pParams=p.bytes.empty()?nullptr:p.bytes.data();kernels.push_back(p.kernel);}
 inside=true;auto status=chain(current,kernels.data(),static_cast<NvU32>(kernels.size()));inside=false;
 log<<frame<<"\tgroup\t"<<kernels.size()<<"\t"<<status<<"\n";++groups;
 pending.clear();if(status!=NVAPI_OK){log.flush();exit(61);}
}
static void Boundary(ID3D12GraphicsCommandList*c,const char*name){
 if(!active||inside)return;
 Flush();log<<frame<<"\td3d\t"<<name<<"\t0\n";
}
#include "launch_boundaries.h"
static HRESULT STDMETHODCALLTYPE OnQuery(ID3D12GraphicsCommandList*c,REFIID id,void**out){
 if(active&&!inside&&id!=__uuidof(IUnknown)&&id!=__uuidof(ID3D12Object)&&id!=__uuidof(ID3D12DeviceChild)&&id!=__uuidof(ID3D12CommandList)&&id!=__uuidof(ID3D12GraphicsCommandList)){
  // Extended command-list methods are not intercepted by this isolated prototype.
  log<<frame<<"\tunsupported_interface\t0\t0\n";log.flush();exit(62);
 }
 return queryInterface(c,id,out);
}
static NvAPI_Status __cdecl OnChain(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS*k,NvU32 n){
 if(!active){return chain(c,k,n);}
 if(c!=current||!k||!n||n>256)exit(63);
 launches+=n;
 if(!batch){inside=true;auto r=chain(c,k,n);inside=false;log<<frame<<"\tgroup\t"<<n<<"\t"<<r<<"\n";++groups;return r;}
 for(unsigned i=0;i<n;i++){
  if(k[i].paramSize>4096||(k[i].paramSize&&!k[i].pParams))exit(64);
  Pending p{};p.kernel=k[i];if(k[i].paramSize){auto bytes=static_cast<const unsigned char*>(k[i].pParams);p.bytes.assign(bytes,bytes+k[i].paramSize);}pending.push_back(std::move(p));
  if(pending.size()==limit)Flush();
 }
 return NVAPI_OK;
}
static NvAPI_Status __cdecl OnChainEx(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS_EX*k,NvU32 n){
 if(active){Flush();log<<frame<<"\tex_unbatched\t"<<n<<"\t0\n";}
 inside=true;auto r=chainEx(c,k,n);inside=false;return r;
}
static void Begin(unsigned f,ID3D12GraphicsCommandList*c){frame=f;current=c;launches=groups=0;active=true;}
static void End(){Flush();active=false;log<<frame<<"\tframe_launches\t"<<launches<<"\t"<<groups<<"\n";log.flush();}
static void Install(ID3D12GraphicsCommandList*c){
 char configured[16]{};if(GetEnvironmentVariableA("NRSTUDIO_BATCH_LIMIT",configured,16)){limit=atoi(configured);if(limit!=4&&limit!=16&&limit!=256)exit(66);}
 char mode[8]{};GetEnvironmentVariableA("NRSTUDIO_BATCH",mode,8);batch=!strcmp(mode,"1");
 auto nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!nv)exit(20);
 auto q=reinterpret_cast<void*(__cdecl*)(unsigned)>(GetProcAddress(nv,"nvapi_QueryInterface"));if(!q)exit(21);
 chain=reinterpret_cast<Chain>(q(0x24973538));chainEx=reinterpret_cast<ChainEx>(q(0x846a9bf0));if(!chain||!chainEx)exit(22);
 auto v=*reinterpret_cast<void***>(c);queryInterface=reinterpret_cast<Query>(v[0]);
 for(unsigned i=9;i<60;i++)for(unsigned j=i+1;j<60;j++)if(v[i]==v[j]){printf("Aliased vtable entries %u %u; refusing hooks\n",i,j);exit(65);}
 log.open("batch-events.tsv");log<<"frame\tevent\tvalue\tstatus_or_groups\n";
 if(DetourTransactionBegin()!=NO_ERROR)exit(23);DetourUpdateThread(GetCurrentThread());
 AttachD3D(c);
 if(DetourAttach(reinterpret_cast<PVOID*>(&chain),OnChain)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&chainEx),OnChainEx)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&queryInterface),OnQuery)!=NO_ERROR){DetourTransactionAbort();exit(24);}
 if(DetourTransactionCommit()!=NO_ERROR)exit(25);
 printf("Isolated batch mode=%d; command-list boundaries retained\n",batch);
}
}
