#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <cstdio>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <DirectXPackedVector.h>
void check(HRESULT h){if(FAILED(h)){printf("HRESULT=%08x\n",(unsigned)h);exit(2);}}
std::vector<char> bytes(const char* p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void transition(ID3D12GraphicsCommandList*c,ID3D12Resource*r,D3D12_RESOURCE_STATES a,D3D12_RESOURCE_STATES b){D3D12_RESOURCE_BARRIER x={};x.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;x.Transition={r,D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,a,b};c->ResourceBarrier(1,&x);}
struct Constants {unsigned mode=1;float white=1;unsigned width=2560,height=1440;float strength=.75f,colour=.5f;unsigned debug=0;float ratio=2;unsigned passthrough=0;float mvx=1,mvy=1;unsigned guidew=2560,guideh=1440,compare=0;float split=.5f,zoom=1;unsigned swap=0;};
int main(int argc,char**argv){
 if(argc<3)return 2;
 const unsigned W=2560,H=1440;const bool half=argc>3;const unsigned pixelBytes=half?8:16;ID3D12Device*d=nullptr;check(D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,IID_PPV_ARGS(&d)));
 ID3D12CommandQueue*q=nullptr;D3D12_COMMAND_QUEUE_DESC qd={};check(d->CreateCommandQueue(&qd,IID_PPV_ARGS(&q)));
 ID3D12CommandAllocator*a=nullptr;check(d->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(&a)));
 ID3D12GraphicsCommandList*c=nullptr;check(d->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,a,nullptr,IID_PPV_ARGS(&c)));check(c->Close());
 ID3D12Fence*f=nullptr;check(d->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&f)));HANDLE e=CreateEvent(nullptr,FALSE,FALSE,nullptr);UINT64 value=0;
 auto begin=[&](){check(a->Reset());check(c->Reset(a,nullptr));};
 auto finish=[&](){check(c->Close());ID3D12CommandList*l[]={c};q->ExecuteCommandLists(1,l);check(q->Signal(f,++value));check(f->SetEventOnCompletion(value,e));if(WaitForSingleObject(e,30000)!=WAIT_OBJECT_0)exit(3);check(d->GetDeviceRemovedReason());};
 auto buffer=[&](UINT64 size,D3D12_HEAP_TYPE type){D3D12_HEAP_PROPERTIES hp={};hp.Type=type;D3D12_RESOURCE_DESC rd={};rd.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;rd.Width=size;rd.Height=1;rd.DepthOrArraySize=1;rd.MipLevels=1;rd.SampleDesc.Count=1;rd.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;ID3D12Resource*r=nullptr;check(d->CreateCommittedResource(&hp,D3D12_HEAP_FLAG_NONE,&rd,type==D3D12_HEAP_TYPE_UPLOAD?D3D12_RESOURCE_STATE_GENERIC_READ:D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&r)));return r;};
 D3D12_DESCRIPTOR_RANGE ranges[2]={{D3D12_DESCRIPTOR_RANGE_TYPE_SRV,3,0,0,0},{D3D12_DESCRIPTOR_RANGE_TYPE_UAV,2,0,0,0}};
 D3D12_ROOT_PARAMETER rp[3]={};rp[0].ParameterType=D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;rp[0].Constants={0,0,sizeof(Constants)/4};
 for(int i=0;i<2;i++){rp[i+1].ParameterType=D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;rp[i+1].DescriptorTable={1,&ranges[i]};}
 D3D12_STATIC_SAMPLER_DESC sampler={};sampler.Filter=D3D12_FILTER_MIN_MAG_MIP_LINEAR;sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D12_TEXTURE_ADDRESS_MODE_CLAMP;sampler.MaxLOD=D3D12_FLOAT32_MAX;sampler.ShaderVisibility=D3D12_SHADER_VISIBILITY_ALL;
 D3D12_ROOT_SIGNATURE_DESC sd={3,rp,1,&sampler,D3D12_ROOT_SIGNATURE_FLAG_NONE};ID3DBlob*rs=nullptr,*err=nullptr;check(D3D12SerializeRootSignature(&sd,D3D_ROOT_SIGNATURE_VERSION_1,&rs,&err));ID3D12RootSignature*sig=nullptr;check(d->CreateRootSignature(0,rs->GetBufferPointer(),rs->GetBufferSize(),IID_PPV_ARGS(&sig)));
 ID3D12PipelineState*pso[2]={};for(int i=0;i<2;i++){auto b=bytes(argv[i+1]);D3D12_COMPUTE_PIPELINE_STATE_DESC pd={};pd.pRootSignature=sig;pd.CS={b.data(),b.size()};check(d->CreateComputePipelineState(&pd,IID_PPV_ARGS(&pso[i])));}
 ID3D12DescriptorHeap*heap=nullptr;D3D12_DESCRIPTOR_HEAP_DESC hd={D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,7,D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,0};check(d->CreateDescriptorHeap(&hd,IID_PPV_ARGS(&heap)));UINT stride=d->GetDescriptorHandleIncrementSize(hd.Type);
 auto cpu=[&](int n){auto x=heap->GetCPUDescriptorHandleForHeapStart();x.ptr+=n*stride;return x;};auto gpu=[&](int n){auto x=heap->GetGPUDescriptorHandleForHeapStart();x.ptr+=n*stride;return x;};
 ID3D12Resource*t[5]={};ID3D12Resource*up[3]={};D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp={};UINT64 size=0;
 begin();
 for(int i=0;i<5;i++){
  D3D12_HEAP_PROPERTIES hp={};hp.Type=D3D12_HEAP_TYPE_DEFAULT;D3D12_RESOURCE_DESC td={};td.Dimension=D3D12_RESOURCE_DIMENSION_TEXTURE2D;td.Width=W;td.Height=H;td.DepthOrArraySize=1;td.MipLevels=1;td.Format=half?DXGI_FORMAT_R16G16B16A16_FLOAT:DXGI_FORMAT_R32G32B32A32_FLOAT;td.SampleDesc.Count=1;if(i>=3)td.Flags=D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  check(d->CreateCommittedResource(&hp,D3D12_HEAP_FLAG_NONE,&td,i<3?D3D12_RESOURCE_STATE_COPY_DEST:D3D12_RESOURCE_STATE_UNORDERED_ACCESS,nullptr,IID_PPV_ARGS(&t[i])));
  d->GetCopyableFootprints(&td,0,1,0,&fp,nullptr,nullptr,&size);
  if(i<3){
   up[i]=buffer(size,D3D12_HEAP_TYPE_UPLOAD);char*ptr=nullptr;check(up[i]->Map(0,nullptr,(void**)&ptr));
   for(unsigned y=0;y<H;y++)for(unsigned x=0;x<W;x++){
    float v[4];unsigned seed=(x+y*W)*747796405u+2891336453u;
    for(int k=0;k<3;k++){seed=seed*1664525u+1013904223u;float r=float(seed&65535)/65535.f;v[k]=i==2?r*r*16.f:r;}
    if(x%31==0)v[0]=v[1]=v[2]=float(y)/H; // Neutral colours and near-zero chroma.
    if(x%37==0)v[0]=v[1]=v[2]=0;
    if(x%41==0){v[0]=1;v[1]=v[2]=0;}
    v[3]=float(x%256)/255.f;if(half){unsigned short packed[4];for(int k=0;k<4;k++)packed[k]=DirectX::PackedVector::XMConvertFloatToHalf(v[k]);memcpy(ptr+y*fp.Footprint.RowPitch+x*pixelBytes,packed,pixelBytes);}else memcpy(ptr+y*fp.Footprint.RowPitch+x*pixelBytes,v,pixelBytes);
   }
   up[i]->Unmap(0,nullptr);D3D12_TEXTURE_COPY_LOCATION src={};src.pResource=up[i];src.Type=D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;src.PlacedFootprint=fp;D3D12_TEXTURE_COPY_LOCATION dst={};dst.pResource=t[i];c->CopyTextureRegion(&dst,0,0,0,&src,nullptr);transition(c,t[i],D3D12_RESOURCE_STATE_COPY_DEST,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
   D3D12_SHADER_RESOURCE_VIEW_DESC srv={};srv.Format=td.Format;srv.ViewDimension=D3D12_SRV_DIMENSION_TEXTURE2D;srv.Shader4ComponentMapping=D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;srv.Texture2D.MipLevels=1;d->CreateShaderResourceView(t[i],&srv,cpu(i));
  }else{D3D12_UNORDERED_ACCESS_VIEW_DESC u={};u.Format=td.Format;u.ViewDimension=D3D12_UAV_DIMENSION_TEXTURE2D;d->CreateUnorderedAccessView(t[i],nullptr,&u,cpu(3+(i-3)*2));d->CreateUnorderedAccessView(t[i],nullptr,&u,cpu(4+(i-3)*2));}
 }
 finish();auto read=buffer(size,D3D12_HEAP_TYPE_READBACK);std::vector<float> baseline(W*H*4);
 D3D12_QUERY_HEAP_DESC qhd={D3D12_QUERY_HEAP_TYPE_TIMESTAMP,2,0};ID3D12QueryHeap*qh=nullptr;check(d->CreateQueryHeap(&qhd,IID_PPV_ARGS(&qh)));auto times=buffer(16,D3D12_HEAP_TYPE_READBACK);UINT64 frequency=0;check(q->GetTimestampFrequency(&frequency));
 auto bind=[&](int n,Constants& params){c->SetDescriptorHeaps(1,&heap);c->SetComputeRootSignature(sig);c->SetPipelineState(pso[n]);c->SetComputeRoot32BitConstants(0,sizeof(params)/4,&params,0);c->SetComputeRootDescriptorTable(1,gpu(0));c->SetComputeRootDescriptorTable(2,gpu(3+n*2));};
 double maximum=0,relative=0,totalSquared=0;UINT64 elements=0,invalid=0,alphaMismatch=0;
 for(int test=0;test<8;test++){
  Constants params;params.passthrough=test&1;params.strength=test==2?0:test==3?2:.75f;params.white=test==4?.25f:test==5?4:1;params.compare=test==6?1:test==7?2:0;
  for(int n=0;n<2;n++){
   begin();bind(n,params);c->Dispatch((W+7)/8,(H+7)/8,1);transition(c,t[3+n],D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COPY_SOURCE);D3D12_TEXTURE_COPY_LOCATION src={};src.pResource=t[3+n];D3D12_TEXTURE_COPY_LOCATION dst={};dst.pResource=read;dst.Type=D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;dst.PlacedFootprint=fp;c->CopyTextureRegion(&dst,0,0,0,&src,nullptr);transition(c,t[3+n],D3D12_RESOURCE_STATE_COPY_SOURCE,D3D12_RESOURCE_STATE_UNORDERED_ACCESS);finish();char*ptr=nullptr;check(read->Map(0,nullptr,(void**)&ptr));
   for(unsigned y=0;y<H;y++)for(unsigned x=0;x<W*4;x++){float v=half?DirectX::PackedVector::XMConvertHalfToFloat(((unsigned short*)(ptr+y*fp.Footprint.RowPitch))[x]):((float*)(ptr+y*fp.Footprint.RowPitch))[x];size_t j=size_t(y)*W*4+x;if(n==0)baseline[j]=v;else{double delta=fabs(double(v)-baseline[j]);maximum=std::max(maximum,delta);relative=std::max(relative,delta/std::max(1.f,fabsf(baseline[j])));totalSquared+=delta*delta;invalid+=!std::isfinite(v);alphaMismatch+=(x%4==3 && v!=baseline[j]);elements++;}}
   read->Unmap(0,nullptr);
  }
 }
 printf("comparison max_abs=%.9g max_scaled_error=%.9g rms=%.9g invalid=%llu alpha_mismatch=%llu elements=%llu\n",maximum,relative,sqrt(totalSquared/elements),invalid,alphaMismatch,elements);
 Constants params;
 for(int run=0;run<6;run++)for(int order=0;order<2;order++){
  int n=(run&1)?1-order:order;begin();bind(n,params);c->Dispatch((W+7)/8,(H+7)/8,1);D3D12_RESOURCE_BARRIER u={};u.Type=D3D12_RESOURCE_BARRIER_TYPE_UAV;u.UAV.pResource=t[3+n];c->ResourceBarrier(1,&u);c->EndQuery(qh,D3D12_QUERY_TYPE_TIMESTAMP,0);
  for(int j=0;j<100;j++){c->Dispatch((W+7)/8,(H+7)/8,1);c->ResourceBarrier(1,&u);}
  c->EndQuery(qh,D3D12_QUERY_TYPE_TIMESTAMP,1);c->ResolveQueryData(qh,D3D12_QUERY_TYPE_TIMESTAMP,0,2,times,0);finish();UINT64*stamp=nullptr;check(times->Map(0,nullptr,(void**)&stamp));printf("timing run=%d variant=%s gpu_ms=%.9f\n",run,n?"candidate":"baseline",double(stamp[1]-stamp[0])/frequency*1000/100);times->Unmap(0,nullptr);
 }
 return invalid||alphaMismatch||relative>0.001?1:0;
}
