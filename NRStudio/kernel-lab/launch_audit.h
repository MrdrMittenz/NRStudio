#pragma once
#include <nvapi.h>
#include <detours.h>
#include <fstream>
namespace LaunchAudit {
using Chain=decltype(&NvAPI_D3D12_LaunchCuKernelChain);
using ChainEx=decltype(&NvAPI_D3D12_LaunchCuKernelChainEx);
using Barrier=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*,UINT,const D3D12_RESOURCE_BARRIER*);
static Chain chain;static ChainEx chainEx;static Barrier barrier;
static bool active=false,insideLaunch=false;
static unsigned frame=0,order=0;
static std::ofstream log;
static void Event(const char*kind,unsigned count){if(active)log<<frame<<'\t'<<order++<<'\t'<<kind<<'\t'<<count<<'\n';}
static NvAPI_Status __cdecl OnChain(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS*k,NvU32 n){
 Event("chain",n);insideLaunch=true;auto r=chain(c,k,n);insideLaunch=false;return r;
}
static NvAPI_Status __cdecl OnChainEx(ID3D12GraphicsCommandList*c,const NVAPI_CU_KERNEL_LAUNCH_PARAMS_EX*k,NvU32 n){
 Event("chain_ex",n);insideLaunch=true;auto r=chainEx(c,k,n);insideLaunch=false;return r;
}
static void STDMETHODCALLTYPE OnBarrier(ID3D12GraphicsCommandList*c,UINT n,const D3D12_RESOURCE_BARRIER*b){
 Event(insideLaunch?"driver_barrier":"model_barrier",n);
 if(active)for(UINT i=0;i<n;i++)log<<frame<<'\t'<<order++<<'\t'<<(b[i].Type==D3D12_RESOURCE_BARRIER_TYPE_UAV?"uav":b[i].Type==D3D12_RESOURCE_BARRIER_TYPE_TRANSITION?"transition":"alias")<<'\t'<<b[i].Flags<<'\n';
 barrier(c,n,b);
}
static void Begin(unsigned f){frame=f;order=0;active=true;}
static void End(){active=false;log.flush();}
static void Install(ID3D12GraphicsCommandList*c){
 auto nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!nv)exit(20);
 auto query=reinterpret_cast<void*(__cdecl*)(unsigned)>(GetProcAddress(nv,"nvapi_QueryInterface"));if(!query)exit(21);
 chain=reinterpret_cast<Chain>(query(0x24973538));chainEx=reinterpret_cast<ChainEx>(query(0x846a9bf0));
 barrier=reinterpret_cast<Barrier>((*reinterpret_cast<void***>(c))[26]);
 if(!chain||!chainEx||!barrier)exit(22);
 log.open("launch-order.tsv");log<<"frame\torder\tevent\tcount_or_flags\n";
 if(DetourTransactionBegin()!=NO_ERROR)exit(23);DetourUpdateThread(GetCurrentThread());
 if(DetourAttach(reinterpret_cast<PVOID*>(&chain),OnChain)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&chainEx),OnChainEx)!=NO_ERROR||DetourAttach(reinterpret_cast<PVOID*>(&barrier),OnBarrier)!=NO_ERROR){DetourTransactionAbort();exit(24);}
 if(DetourTransactionCommit()!=NO_ERROR)exit(25);
}
}
