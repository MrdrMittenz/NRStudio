#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>

template<class T> T symbol(HMODULE m, const char* n) {
    auto p = GetProcAddress(m,n);
    if (!p) { printf("missing %s error=%lu\n",n,GetLastError()); exit(2); }
    return reinterpret_cast<T>(p);
}
void hr(HRESULT h,const char* stage) { if(FAILED(h)){printf("%s HRESULT=%08x\n",stage,(unsigned)h);exit(3);} }
void barrier(ID3D12GraphicsCommandList* cmd,ID3D12Resource* r,D3D12_RESOURCE_STATES a,D3D12_RESOURCE_STATES b) {
    D3D12_RESOURCE_BARRIER x={};x.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;x.Transition={r,D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,a,b};cmd->ResourceBarrier(1,&x);
}
ID3D12Resource* buffer(ID3D12Device*dev,UINT64 size,D3D12_HEAP_TYPE type) {
    D3D12_HEAP_PROPERTIES h={};h.Type=type;D3D12_RESOURCE_DESC d={};d.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;d.Width=size;d.Height=1;d.DepthOrArraySize=1;d.MipLevels=1;d.SampleDesc.Count=1;d.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ID3D12Resource*r=nullptr;hr(dev->CreateCommittedResource(&h,D3D12_HEAP_FLAG_NONE,&d,type==D3D12_HEAP_TYPE_UPLOAD?D3D12_RESOURCE_STATE_GENERIC_READ:D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&r)),"buffer");return r;
}
float half(uint16_t v) { int e=(v>>10)&31;float m=float(v&1023);float f=e==31?(m?NAN:INFINITY):e?std::ldexp(1.f+m/1024.f,e-15):std::ldexp(m,-24);return v&32768?-f:f; }
#include "launch_audit.h"
int wmain(int argc,wchar_t**argv) {
    setvbuf(stdout,nullptr,_IONBF,0);
    if(argc<4) { puts("usage: native_probe core.dll model.dll forwarder.dll [width height frames]");return 2; }
    const unsigned W=argc>4?_wtoi(argv[4]):256,H=argc>5?_wtoi(argv[5]):256;
    const int frames=argc>6?_wtoi(argv[6]):3;
    if(!W||!H||W>3840||H>2160||frames<1||frames>1200)return 2;
    ID3D12Device* dev=nullptr;
    hr(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,IID_PPV_ARGS(&dev)),"device");
    ID3D12CommandQueue* queue=nullptr; D3D12_COMMAND_QUEUE_DESC qd={};
    hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");
    ID3D12CommandAllocator* alloc=nullptr;
    hr(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(&alloc)),"allocator");
    ID3D12GraphicsCommandList* cmd=nullptr;
    hr(dev->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,alloc,nullptr,IID_PPV_ARGS(&cmd)),"command list");
    LaunchAudit::Install(cmd);
    auto core=LoadLibraryExW(argv[1],nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
    auto fwd=LoadLibraryExW(argv[3],nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
    if(!core||!fwd){printf("load error %lu\n",GetLastError());return 2;}
    using Init=int(*)(unsigned long long,const wchar_t*,ID3D12Device*,int,const void*);
    int rc=symbol<Init>(core,"NVSDK_NGX_D3D12_Init_Ext")(0x24480451,L".",dev,0x15,nullptr);
    printf("core init=%08x\n",rc); if(rc!=1)return 4;
    void* params=nullptr;
    rc=symbol<int(*)(void**)>(core,"NVSDK_NGX_D3D12_GetCapabilityParameters")(&params);
    printf("capabilities=%08x params=%p\n",rc,params);if(rc!=1||!params)return 5;
    void**vt=*reinterpret_cast<void***>(params);
    // MSVC lays these overloaded methods out in reverse declaration order.
    using SetFloat=void(*)(void*,const char*,float);
    using GetFloat=int(*)(void*,const char*,float*);
    reinterpret_cast<SetFloat>(vt[6])(params,"NRProbe.Float",0.3125f);
    float roundtrip=0;
    rc=reinterpret_cast<GetFloat>(vt[14])(params,"NRProbe.Float",&roundtrip);
    printf("float slot6 rc=%08x value=%g\n",rc,roundtrip);if(rc!=1||roundtrip!=0.3125f)return 6;
    symbol<void(*)(int)>(fwd,"dlssnr_call_set_float_slot")(6);
    using Create=void*(*)(const wchar_t*,const wchar_t*,ID3D12Device*,ID3D12GraphicsCommandList*,void*,unsigned,unsigned,int,float,int,float,float,float,int,int);
    void* feature=symbol<Create>(fwd,"dlssnr_call_create")(argv[2],L".",dev,cmd,params,W,H,0,1.f,0,1.f,1.f,1.f,1,0);
    printf("native init=%08x create=%08x feature=%p\n",*symbol<int*>(fwd,"dlssnr_call_last_init"),*symbol<int*>(fwd,"dlssnr_call_last_create"),feature);
    if(!feature)return 7;
    hr(cmd->Close(),"close");ID3D12CommandList* lists[]={cmd};queue->ExecuteCommandLists(1,lists);
    ID3D12Fence* fence=nullptr;hr(dev->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&fence)),"fence");
    HANDLE done=CreateEventW(nullptr,FALSE,FALSE,nullptr);hr(queue->Signal(fence,1),"signal");hr(fence->SetEventOnCompletion(1,done),"event");
    if(WaitForSingleObject(done,30000)!=WAIT_OBJECT_0){puts("GPU initialization timeout");return 8;}
    hr(dev->GetDeviceRemovedReason(),"device after initialization");
    puts("native feature initialization submitted and completed");
    hr(alloc->Reset(),"allocator reset");hr(cmd->Reset(alloc,nullptr),"list reset");
    ID3D12Resource* tex[4]={};ID3D12Resource* uploads[3]={};
    DXGI_FORMAT formats[]={DXGI_FORMAT_R16G16B16A16_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R16G16_FLOAT,DXGI_FORMAT_R16G16B16A16_FLOAT};
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprints[4]={};UINT64 sizes[4]={};
    for(int i=0;i<4;i++) {
        D3D12_HEAP_PROPERTIES hp={};hp.Type=D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC d={};d.Dimension=D3D12_RESOURCE_DIMENSION_TEXTURE2D;d.Width=W;d.Height=H;d.DepthOrArraySize=1;d.MipLevels=1;d.Format=formats[i];d.SampleDesc.Count=1;d.Flags=i==3?D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS:D3D12_RESOURCE_FLAG_NONE;
        hr(dev->CreateCommittedResource(&hp,D3D12_HEAP_FLAG_NONE,&d,i==3?D3D12_RESOURCE_STATE_UNORDERED_ACCESS:D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&tex[i])),"texture");
        dev->GetCopyableFootprints(&d,0,1,0,&footprints[i],nullptr,nullptr,&sizes[i]);
        if(i==3)continue;
        uploads[i]=buffer(dev,sizes[i],D3D12_HEAP_TYPE_UPLOAD);void* ptr=nullptr;hr(uploads[i]->Map(0,nullptr,&ptr),"upload map");memset(ptr,0,(size_t)sizes[i]);
        for(unsigned y=0;y<H;y++)for(unsigned x=0;x<W;x++){
            auto row=(unsigned char*)ptr+y*footprints[i].Footprint.RowPitch;
            if(i==0){uint16_t pixel[]={uint16_t(0x3000+(x*8/W)*128),uint16_t(0x3400+(y*8/H)*128),0x3800,0x3c00};memcpy(row+x*8,pixel,8);}
            if(i==1){float depth=.5f;memcpy(row+x*4,&depth,4);}
        }
        uploads[i]->Unmap(0,nullptr);
        D3D12_TEXTURE_COPY_LOCATION src={};src.pResource=uploads[i];src.Type=D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;src.PlacedFootprint=footprints[i];
        D3D12_TEXTURE_COPY_LOCATION dst={};dst.pResource=tex[i];dst.Type=D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        cmd->CopyTextureRegion(&dst,0,0,0,&src,nullptr);barrier(cmd,tex[i],D3D12_RESOURCE_STATE_COPY_DEST,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    auto readback=buffer(dev,sizes[3],D3D12_HEAP_TYPE_READBACK);
    using Evaluate=int(*)(ID3D12GraphicsCommandList*,void*,void*,ID3D12Resource*,ID3D12Resource*,ID3D12Resource*,ID3D12Resource*,unsigned,unsigned,unsigned,unsigned,int,int,float,int,float,float,float,int,float,float);
    auto evaluate=symbol<Evaluate>(fwd,"dlssnr_call_evaluate");
    for(int frame=0;frame<frames;frame++){
        if(frame){hr(alloc->Reset(),"allocator reset");hr(cmd->Reset(alloc,nullptr),"list reset");barrier(cmd,tex[3],D3D12_RESOURCE_STATE_COPY_SOURCE,D3D12_RESOURCE_STATE_UNORDERED_ACCESS);}
        LARGE_INTEGER begin,end,freq;QueryPerformanceFrequency(&freq);QueryPerformanceCounter(&begin);
        LaunchAudit::Begin(frame);
        rc=evaluate(cmd,feature,params,tex[0],tex[1],tex[2],tex[3],W,H,W,H,1,frame==0,1.f,0,1.f,1.f,1.f,1,1.f,1.f);
        LaunchAudit::End();
        printf("frame %d evaluate=%08x\n",frame,rc);if(rc!=1)return 9;
        barrier(cmd,tex[3],D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COPY_SOURCE);
        D3D12_TEXTURE_COPY_LOCATION src={};src.pResource=tex[3];src.Type=D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        D3D12_TEXTURE_COPY_LOCATION dst={};dst.pResource=readback;dst.Type=D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;dst.PlacedFootprint=footprints[3];cmd->CopyTextureRegion(&dst,0,0,0,&src,nullptr);
        hr(cmd->Close(),"close evaluation");queue->ExecuteCommandLists(1,lists);hr(queue->Signal(fence,frame+2),"signal evaluation");hr(fence->SetEventOnCompletion(frame+2,done),"event evaluation");
        if(WaitForSingleObject(done,30000)!=WAIT_OBJECT_0){puts("GPU evaluation timeout");return 10;}hr(dev->GetDeviceRemovedReason(),"device after evaluation");
        QueryPerformanceCounter(&end);printf("evaluation and readback completed in %.3f ms at %ux%u\n",1000.0*(end.QuadPart-begin.QuadPart)/freq.QuadPart,W,H);
        void* p=nullptr;hr(readback->Map(0,nullptr,&p),"readback");double sum=0,delta=0;unsigned invalid=0;float lo=INFINITY,hi=-INFINITY;
        for(unsigned y=0;y<H;y++)for(unsigned x=0;x<W;x++)for(int c=0;c<4;c++){
            float v=half(((uint16_t*)((unsigned char*)p+y*footprints[3].Footprint.RowPitch))[x*4+c]);
            if(!std::isfinite(v)){invalid++;continue;}lo=fminf(lo,v);hi=fmaxf(hi,v);sum+=v;
            uint16_t expected[]={uint16_t(0x3000+(x*8/W)*128),uint16_t(0x3400+(y*8/H)*128),0x3800,0x3c00};delta+=fabs(v-half(expected[c]));
        }
        printf("frame %d invalid=%u range=[%g,%g] sum=%.9g mean_abs_change=%.9g\n",frame,invalid,lo,hi,sum,delta/(W*H*4));
        if(frame==frames-1){FILE* file=nullptr;char filename[96];sprintf_s(filename,"native-output-%ux%u.rgba16f",W,H);fopen_s(&file,filename,"wb");if(file){for(unsigned y=0;y<H;y++)fwrite((unsigned char*)p+y*footprints[3].Footprint.RowPitch,1,W*8,file);fclose(file);}}readback->Unmap(0,nullptr);
        if(invalid||delta==0||sum==0)return 11;
    }
    symbol<void(*)(void*)>(fwd,"dlssnr_call_release")(feature);
    return 0;
}
