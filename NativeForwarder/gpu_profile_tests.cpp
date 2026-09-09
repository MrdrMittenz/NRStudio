#include <dxgi1_4.h>
#include <cassert>
#include <cstdio>
#include "gpu_profile.h"
static NvAPI_Status __cdecl Init(){return NVAPI_OK;}
static NvAPI_Status __cdecl Enum(NvLogicalGpuHandle* handles,NvU32* count){handles[0]=(NvLogicalGpuHandle)1;handles[1]=(NvLogicalGpuHandle)2;*count=2;return NVAPI_OK;}
static NvAPI_Status __cdecl Info(NvLogicalGpuHandle h,NV_LOGICAL_GPU_DATA* data){
    auto luid=(LUID*)data->pOSAdapterId;luid->LowPart=(DWORD)(uintptr_t)h;luid->HighPart=7;
    data->physicalGpuCount=1;data->physicalGpuHandles[0]=(NvPhysicalGpuHandle)h;return NVAPI_OK;
}
static NvAPI_Status __cdecl Arch(NvPhysicalGpuHandle h,NV_GPU_ARCH_INFO* a){a->architecture=h==(NvPhysicalGpuHandle)1?NV_GPU_ARCHITECTURE_GA100:NV_GPU_ARCHITECTURE_AD100;return NVAPI_OK;}
static void* __cdecl FakeQuery(unsigned id){switch(id){case 0x0150e828:return (void*)&Init;case 0x48b3ea59:return (void*)&Enum;case 0x842b066e:return (void*)&Info;case 0xd8265d24:return (void*)&Arch;default:return nullptr;}}
int main(){
    using namespace NRGpu;
    assert(Select(0x10de,0x2204,0)==Profile::Ampere3090);
    assert(Select(0x10de,0x2684,NV_GPU_ARCHITECTURE_AD100)==Profile::Ada);
    assert(Select(0x10de,0x2882,NV_GPU_ARCHITECTURE_AD100)==Profile::Ada);
    assert(Select(0x10de,0x2684,0)==Profile::Unsupported);
    assert(Select(0x1002,0x2204,NV_GPU_ARCHITECTURE_AD100)==Profile::Unsupported);
    assert(Select(0x10de,0x2b85,0x1b0)==Profile::Unsupported);
    assert(Architecture(LUID{2,7},FakeQuery)==NV_GPU_ARCHITECTURE_AD100);
    assert(Architecture(LUID{1,7},FakeQuery)==NV_GPU_ARCHITECTURE_GA100);
    assert(Architecture(LUID{2,8},FakeQuery)==0);
    assert(Architecture(LUID{2,7},nullptr)==0);
    puts("PASS profile selection, unknown adapters and exact multi-GPU LUID matching");
    auto nv=LoadLibraryExW(L"nvapi64.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);assert(nv);
    auto query=(NRGpu::Query)GetProcAddress(nv,"nvapi_QueryInterface");assert(query);
    IDXGIFactory4* f=nullptr;assert(SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&f))));
    bool found=false;
    for(unsigned i=0;;i++){
        IDXGIAdapter1* a=nullptr;if(f->EnumAdapters1(i,&a)==DXGI_ERROR_NOT_FOUND)break;
        DXGI_ADAPTER_DESC1 d{};a->GetDesc1(&d);a->Release();
        if(d.VendorId!=0x10de)continue;
        unsigned arch=Architecture(d.AdapterLuid,query);assert(arch!=0);found=true;
        printf("Live NVIDIA device=%04x architecture=%x profile=%d\n",d.DeviceId,arch,(int)Select(d.VendorId,d.DeviceId,arch));
        if(d.DeviceId==0x2204)assert(Select(d.VendorId,d.DeviceId,arch)==Profile::Ampere3090);
    }
    f->Release();assert(found);puts("PASS live NVIDIA adapter detection");
}
