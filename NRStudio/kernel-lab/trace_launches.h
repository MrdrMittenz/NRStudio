#include <nvapi.h>
#include <detours.h>
#include <map>
#include <string>
#include <mutex>
#include <fstream>
#include <iomanip>
#include "chain_timing.h"
namespace NRTrace {
using Create=decltype(&NvAPI_D3D12_CreateCuFunction);
using Chain=decltype(&NvAPI_D3D12_LaunchCuKernelChain);
using ChainEx=decltype(&NvAPI_D3D12_LaunchCuKernelChainEx);
static Create create=nullptr;static Chain chain=nullptr;static ChainEx chainEx=nullptr;
static std::map<NVDX_ObjectHandle,std::string> names;
static std::map<std::string,unsigned> counts;
static std::mutex lock;
static std::ofstream trace;
static NvAPI_Status __cdecl OnCreate(ID3D12Device*d,NVDX_ObjectHandle m,const char*n,NVDX_ObjectHandle*out){
 auto r=create(d,m,n,out);if(r==NVAPI_OK&&out&&n){std::lock_guard<std::mutex>guard(lock);names[*out]=n;}return r;
}
template<class T> static void Record(const T* kernels,NvU32 count){
 if(!kernels||count>100000)return;
 std::lock_guard<std::mutex>guard(lock);
 for(unsigned i=0;i<count;i++){
  const auto& k=kernels[i];auto it=names.find(k.hFunction);std::string name=it==names.end()?"unknown":it->second;unsigned call=++counts[name];
  if(call>1)continue;
  trace<<name<<"\tgrid="<<k.gridDim.x<<","<<k.gridDim.y<<","<<k.gridDim.z<<"\tblock="<<k.blockDim.x<<","<<k.blockDim.y<<","<<k.blockDim.z<<"\tshared="<<k.dynSharedMemBytes<<"\tbytes="<<k.paramSize<<"\tparams=";
  if(k.pParams&&k.paramSize<=4096){auto b=static_cast<const unsigned char*>(k.pParams);for(unsigned j=0;j<k.paramSize;j++)trace<<std::hex<<std::setw(2)<<std::setfill('0')<<(unsigned)b[j];trace<<std::dec;}
  trace<<"\n";
 }
 trace.flush();
}
template<class T> static unsigned StartTiming(ID3D12GraphicsCommandList*c,const T*k,NvU32 n){
 std::lock_guard<std::mutex>guard(lock);
 return ChainTiming::Begin(c,n,n?names[k[0].hFunction]:"empty",n?names[k[n-1].hFunction]:"empty");
}
static NvAPI_Status __cdecl OnChain(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS*k,NvU32 n){if(!c||!k||!n)return chain(c,k,n);Record(k,n);unsigned index=StartTiming(c,k,n);auto r=chain(c,k,n);ChainTiming::End(c,index);return r;}
static NvAPI_Status __cdecl OnChainEx(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS_EX*k,NvU32 n){if(!c||!k||!n)return chainEx(c,k,n);Record(k,n);unsigned index=StartTiming(c,k,n);auto r=chainEx(c,k,n);ChainTiming::End(c,index);return r;}
static void Finish(){std::ofstream out("kernel-counts.tsv");for(auto& x:counts)out<<x.first<<"\t"<<x.second<<"\n";trace.flush();ChainTiming::Finish();}
static void Install(){
 HMODULE nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!nv){puts("trace: cannot load system NVAPI");exit(20);}
 auto query=reinterpret_cast<void*(__cdecl*)(unsigned)>(GetProcAddress(nv,"nvapi_QueryInterface"));if(!query)exit(21);
 create=reinterpret_cast<Create>(query(0xe2436e22));chain=reinterpret_cast<Chain>(query(0x24973538));chainEx=reinterpret_cast<ChainEx>(query(0x846a9bf0));if(!create||!chain||!chainEx)exit(22);
 trace.open("kernel-launches.tsv");if(!trace)exit(23);
 if(DetourTransactionBegin()!=NO_ERROR)exit(24);
 DetourUpdateThread(GetCurrentThread());
 if(DetourAttach(reinterpret_cast<PVOID*>(&create),OnCreate)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&chain),OnChain)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&chainEx),OnChainEx)!=NO_ERROR){DetourTransactionAbort();exit(25);}
 if(DetourTransactionCommit()!=NO_ERROR)exit(26);
 atexit(Finish);puts("trace: NVAPI launch recording enabled in isolated probe");
}
}
