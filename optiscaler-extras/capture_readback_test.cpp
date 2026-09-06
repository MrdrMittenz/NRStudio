#define wmain unused_native_probe_main
#include "../fp8-ampere-candidate/tools/nr_recovery/native_probe.cpp"
#undef wmain
#include "OptiScaler/dlssnr/DlssNr_Capture.h"
#include <fstream>
#include <iterator>

int main() {
    ID3D12Device* dev=nullptr;hr(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,IID_PPV_ARGS(&dev)),"device");
    ID3D12CommandQueue* q=nullptr;D3D12_COMMAND_QUEUE_DESC qd={};hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&q)),"queue");
    ID3D12CommandAllocator* a=nullptr;hr(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(&a)),"allocator");
    ID3D12GraphicsCommandList* cmd=nullptr;hr(dev->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,a,nullptr,IID_PPV_ARGS(&cmd)),"cmd");
    ID3D12Resource* tex[2]={};ID3D12Resource* up[2]={};
    const uint16_t pixels[2][4]={{0x4400,0x4800,0x3400,0x3c00},{0x4800,0x4c00,0x3800,0x3c00}};
    for(int i=0;i<2;i++) {
        D3D12_RESOURCE_DESC d={};d.Dimension=D3D12_RESOURCE_DIMENSION_TEXTURE2D;d.Width=32;d.Height=16;d.DepthOrArraySize=1;d.MipLevels=1;d.Format=DXGI_FORMAT_R16G16B16A16_FLOAT;d.SampleDesc.Count=1;
        D3D12_HEAP_PROPERTIES hp={};hp.Type=D3D12_HEAP_TYPE_DEFAULT;
        hr(dev->CreateCommittedResource(&hp,D3D12_HEAP_FLAG_NONE,&d,D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&tex[i])),"texture");
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp={};UINT64 size=0;dev->GetCopyableFootprints(&d,0,1,0,&fp,nullptr,nullptr,&size);
        up[i]=buffer(dev,size,D3D12_HEAP_TYPE_UPLOAD);void* p=nullptr;hr(up[i]->Map(0,nullptr,&p),"map");
        for(int y=0;y<16;y++)for(int x=0;x<32;x++)memcpy((char*)p+y*fp.Footprint.RowPitch+x*8,pixels[i],8);
        up[i]->Unmap(0,nullptr);D3D12_TEXTURE_COPY_LOCATION src={};src.pResource=up[i];src.Type=D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;src.PlacedFootprint=fp;
        D3D12_TEXTURE_COPY_LOCATION dst={};dst.pResource=tex[i];dst.Type=D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;cmd->CopyTextureRegion(&dst,0,0,0,&src,nullptr);
        barrier(cmd,tex[i],D3D12_RESOURCE_STATE_COPY_DEST,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    capture::FrameCapture cap;cap.request(1);cap.record(cmd,dev,tex[0],D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,tex[1],D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    hr(cmd->Close(),"close");ID3D12CommandList* lists[]={cmd};q->ExecuteCommandLists(1,lists);
    ID3D12Fence*f=nullptr;hr(dev->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&f)),"fence");HANDLE done=CreateEventW(nullptr,FALSE,FALSE,nullptr);hr(q->Signal(f,1),"signal");hr(f->SetEventOnCompletion(1,done),"event");if(WaitForSingleObject(done,30000)!=WAIT_OBJECT_0)return 1;
    if(cap.write("capture-test-output").empty())return 2;
    for(int i=0;i<2;i++) {
        std::ifstream input(i?"capture-test-output/after_00.raw":"capture-test-output/before_00.raw",std::ios::binary);
        std::vector<char> bytes((std::istreambuf_iterator<char>(input)),{});
        if(bytes.size()!=32*16*8)return 3;
        for(size_t offset=0;offset<bytes.size();offset+=8)if(memcmp(bytes.data()+offset,pixels[i],8)!=0)return 4;
    }
    std::ifstream mf("capture-test-output/manifest.txt");std::string manifest((std::istreambuf_iterator<char>(mf)),{});
    if(manifest.find("capture_version 2")==std::string::npos)return 5;
    puts("PASS: GPU capture preserves distinct HDR values above 1 and alpha byte-for-byte; versioned manifest written.");return 0;
}
