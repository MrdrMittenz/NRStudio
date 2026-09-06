// Included inside NRTrace by prepare_prepared_probe.py; isolated probe only.
#include "prepared_weight_extent.h"
static NVDX_ObjectHandle prepModule{},prepFunction{};
static NVDX_ObjectHandle controlModule{},controlFunction{};
static bool alternating=GetEnvironmentVariableW(L"NRSTUDIO_PREPARED_ALTERNATE",nullptr,0)>0;
static bool UsePrepared(){return !alternating||substitutions%4==1||substitutions%4==2;}
static ID3D12Resource* preparedBuffer=nullptr;
struct PreparedLaunchStorage {
    alignas(8) unsigned char target[192]{};
    struct { uint64_t source,output; unsigned words,padding; } prep{};
};
// Retain parameter bytes until all probe GPU work completes.
static std::vector<PreparedLaunchStorage*> preparedStorage;
static void InitializePrepared(){
 if(preparedBuffer)return;
 wchar_t path[32768];DWORD len=GetEnvironmentVariableW(L"NRSTUDIO_PREP_CUBIN",path,32768);
 if(!len||len>=32768)exit(70);
 std::ifstream file(path,std::ios::binary);
 std::vector<char> blob((std::istreambuf_iterator<char>(file)),{});
 if(blob.empty())exit(71);
 if(createModule(testDevice,blob.data(),static_cast<NvU32>(blob.size()),&prepModule)!=NVAPI_OK)exit(72);
 if(create(testDevice,prepModule,"nrPrepareWeights",&prepFunction)!=NVAPI_OK)exit(73);
 if(alternating){
  len=GetEnvironmentVariableW(L"NRSTUDIO_CONTROL_CUBIN",path,32768);if(!len||len>=32768)exit(76);
  std::ifstream control(path,std::ios::binary);std::vector<char> bytes((std::istreambuf_iterator<char>(control)),{});
  if(bytes.empty()||createModule(testDevice,bytes.data(),static_cast<NvU32>(bytes.size()),&controlModule)!=NVAPI_OK)exit(77);
  if(create(testDevice,controlModule,"cc_tinlayout_fused_post_block_swin_1h_32_fp8",&controlFunction)!=NVAPI_OK)exit(78);
 }
 D3D12_HEAP_PROPERTIES heap{};heap.Type=D3D12_HEAP_TYPE_DEFAULT;
 D3D12_RESOURCE_DESC desc{};desc.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;
 desc.Width=NR_PREPARED_SOURCE_BYTES*2;desc.Height=1;desc.DepthOrArraySize=1;
 desc.MipLevels=1;desc.SampleDesc.Count=1;desc.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
 desc.Flags=D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
 if(FAILED(testDevice->CreateCommittedResource(&heap,D3D12_HEAP_FLAG_NONE,&desc,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,nullptr,IID_PPV_ARGS(&preparedBuffer))))exit(74);
 printf("prepared: source_bytes=%u buffer_bytes=%u refresh=every_evaluation\n",NR_PREPARED_SOURCE_BYTES,NR_PREPARED_SOURCE_BYTES*2);
}
template<class F,class T> static NvAPI_Status LaunchPrepared(F original,ID3D12GraphicsCommandList*c,const T*k,NvU32 n){
 if(!testFunction)return original(c,k,n);
 bool target=false;for(unsigned i=0;i<n;++i)target|=k[i].hFunction==originalFunction;
 if(!target)return original(c,k,n);
 if(n!=1||k[0].paramSize!=184||!k[0].pParams||k[0].blockDim.x!=32||k[0].blockDim.y!=1||k[0].blockDim.z!=1)exit(75);
 InitializePrepared();
 if(!UsePrepared()){
  T control=k[0];control.hFunction=controlFunction;++substitutions;return original(c,&control,1);
 }
 auto* storage=new PreparedLaunchStorage;preparedStorage.push_back(storage);
 memcpy(storage->target,k[0].pParams,184);
 memcpy(&storage->prep.source,storage->target+24,8);
 storage->prep.output=preparedBuffer->GetGPUVirtualAddress();storage->prep.words=NR_PREPARED_SOURCE_BYTES/4;
 memcpy(storage->target+184,&storage->prep.output,8);
 D3D12_RESOURCE_BARRIER barrier{};barrier.Type=D3D12_RESOURCE_BARRIER_TYPE_UAV;
 barrier.UAV.pResource=preparedBuffer;c->ResourceBarrier(1,&barrier);
 T prep=k[0];prep.hFunction=prepFunction;prep.gridDim={(storage->prep.words+127)/128,1,1};
 prep.blockDim={128,1,1};prep.dynSharedMemBytes=0;prep.pParams=&storage->prep;prep.paramSize=sizeof(storage->prep);
 auto result=original(c,&prep,1);if(result!=NVAPI_OK)return result;
 c->ResourceBarrier(1,&barrier);
 T output=k[0];output.hFunction=testFunction;output.pParams=storage->target;output.paramSize=sizeof(storage->target);
 ++substitutions;return original(c,&output,1);
}
static void FinishPrepared(){
 if(!ChainTiming::completed)return;
 if(preparedBuffer)preparedBuffer->Release();
 if(prepFunction)destroyFunction(testDevice,prepFunction);
 if(prepModule)destroyModule(testDevice,prepModule);
 if(controlFunction)destroyFunction(testDevice,controlFunction);
 if(controlModule)destroyModule(testDevice,controlModule);
 for(auto* item:preparedStorage)delete item;
}
