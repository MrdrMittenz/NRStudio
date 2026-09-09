#pragma once
#include <array>
#include <mutex>
#include <vector>
#include <string>
#include <cstdint>
#include <detours.h>
// Bounded process-lifetime storage. A recording keeps its leases until Reset,
// and retired leases are reusable only after every observed submission completes.
namespace NRGpuWork {
constexpr unsigned Capacity=32, QueueCapacity=8, StampCount=8;
using Execute=void(STDMETHODCALLTYPE*)(ID3D12CommandQueue*,UINT,ID3D12CommandList*const*);
using Reset=HRESULT(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList*,ID3D12CommandAllocator*,ID3D12PipelineState*);
static Execute execute=nullptr;static Reset reset=nullptr;
struct Queue {ID3D12CommandQueue* queue{};ID3D12Fence* fence{};UINT64 value{},frequency{};};
struct Slot {
 ID3D12GraphicsCommandList* list{};ID3D12Resource* weights{};
 bool retired{},fault{},submitted{},prepared{},resolved{},reported{},outer{};
 UINT64 id{},submitId{},fenceValue{};Queue* queue{};
 unsigned mask{},width{},height{};
 alignas(8) unsigned char target[192]{};
 struct {UINT64 source,output;unsigned words,pad;} prep{};
};
struct Sample {UINT64 id,submitId;unsigned width,height,mask;bool prepared,outer;double total,encode,guides,model,resolve,post;};
static auto& mutex=*new std::mutex;
static auto& fileMutex=*new std::mutex;
static std::array<Slot,Capacity> slots{};static std::array<Queue,QueueCapacity> queues{};
static auto& samples=*new std::vector<Sample>;
static ID3D12Device* device=nullptr;static ID3D12QueryHeap* queries=nullptr;
static ID3D12Resource* readback=nullptr;static UINT64* mapped=nullptr;
static bool enabled=false,timing=false;
static double latestTotal=-1;
static UINT64 nextId=0,nextSubmit=0,unavailable=0,dropped=0;
static thread_local Slot* current=nullptr;
static auto& csvPath=*new std::wstring;
static bool Done(const Slot&s){return s.submitted&&s.queue&&s.queue->fence&&s.queue->fence->GetCompletedValue()!=UINT64_MAX&&s.queue->fence->GetCompletedValue()>=s.fenceValue;}
static void CollectLocked(){
 for(auto&s:slots){
  if(!s.list||s.fault||!Done(s))continue;
  if(timing&&s.resolved&&!s.reported){
   unsigned index=unsigned(&s-slots.data());const auto* stamps=mapped+index*StampCount;
   auto span=[&](unsigned a,unsigned b){return ((s.mask&(1u<<a))&&(s.mask&(1u<<b))&&stamps[b]>=stamps[a])?double(stamps[b]-stamps[a])*1000.0/s.queue->frequency:-1.0;};
   if(samples.size()<128)samples.push_back({s.id,s.submitId,s.width,s.height,s.mask,s.prepared,s.outer,span(0,4),span(0,1),span(1,2),span(2,3),span(3,4),span(5,6)});else ++dropped;
   if(s.outer&&s.mask==127)latestTotal=span(0,4);
   s.reported=true;
  }
  if(s.retired){auto* weights=s.weights;s=Slot{};s.weights=weights;}
 }
}
static Queue* GetQueueLocked(ID3D12CommandQueue*q){
 for(auto&e:queues)if(e.queue==q)return &e;
 for(auto&e:queues)if(!e.queue){
  ID3D12Device*d=nullptr;if(FAILED(q->GetDevice(IID_PPV_ARGS(&d))))return nullptr;
  bool match=d==device;d->Release();if(!match)return nullptr;
  if(FAILED(device->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&e.fence))))return nullptr;
  if(timing&&(FAILED(q->GetTimestampFrequency(&e.frequency))||!e.frequency)){e.fence->Release();e.fence=nullptr;return nullptr;}
  e.queue=q;q->AddRef();return &e;
 }return nullptr;
}
static void STDMETHODCALLTYPE OnExecute(ID3D12CommandQueue*q,UINT n,ID3D12CommandList*const*lists){
 std::lock_guard<std::mutex> guard(mutex);
 std::array<bool,Capacity> touched{};bool found=false;
 for(unsigned i=0;i<Capacity;++i)if(slots[i].list&&!slots[i].retired)
  for(UINT j=0;j<n;++j)if(slots[i].list==lists[j]){touched[i]=found=true;break;}
 if(!found){execute(q,n,lists);return;}
 CollectLocked();auto* queue=GetQueueLocked(q);
 // Replayed recordings share scratch storage. Order a cross-queue replay behind
 // its previous submission; this is a GPU wait, never a CPU completion wait.
 for(unsigned i=0;i<Capacity;++i)if(touched[i]){
  auto&s=slots[i];
  if(!queue){s.fault=true;continue;}
  if(s.submitted&&s.queue!=queue&&FAILED(q->Wait(s.queue->fence,s.fenceValue)))s.fault=true;
  if(timing&&s.submitted&&!s.reported)++dropped;
 }
 execute(q,n,lists);
 const UINT64 serial=++nextSubmit;
 bool signaled=queue&&SUCCEEDED(q->Signal(queue->fence,++queue->value));
 for(unsigned i=0;i<Capacity;++i)if(touched[i]){
  auto&s=slots[i];if(!signaled){s.fault=true;continue;}
  s.queue=queue;s.fenceValue=queue->value;s.submitted=true;s.submitId=serial;s.reported=false;
 }
}
static HRESULT STDMETHODCALLTYPE OnReset(ID3D12GraphicsCommandList*c,ID3D12CommandAllocator*a,ID3D12PipelineState*p){
 std::lock_guard<std::mutex> guard(mutex);
 auto result=reset(c,a,p);
 if(SUCCEEDED(result))for(auto&s:slots)if(s.list==c&&!s.retired){s.retired=true;if(!s.submitted)s.fault=true;}
 // An unobserved submission is not treated as completed. Quarantine its lease.
 CollectLocked();return result;
}
static void Flush(){
 std::vector<Sample> ready;UINT64 lost=0,miss=0;
 {std::lock_guard<std::mutex> guard(mutex);CollectLocked();ready.swap(samples);lost=dropped;miss=unavailable;}
 if(ready.empty())return;
 std::lock_guard<std::mutex> fileGuard(fileMutex);
 FILE*f=nullptr;_wfopen_s(&f,csvPath.c_str(),L"a");if(!f)return;
 for(const auto&s:ready)fprintf(f,"%llu,%llu,%u,%u,%s,%u,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%llu,%llu\n",s.id,s.submitId,s.width,s.height,s.outer?"pipeline":"model-only",s.prepared?1:0,s.total,s.encode,s.guides,s.model,s.resolve,s.post,lost,miss);
 fclose(f);
}
static DWORD WINAPI Writer(void*){for(;;){Sleep(250);Flush();}}
// Called inside the existing atomic hook transaction. Caller commits all hooks
// before invoking Activate. Dummy objects discover the native method addresses.
static bool Attach(ID3D12Device*d,ID3D12GraphicsCommandList*c,bool wantTiming,const std::wstring&path){
 D3D12_COMMAND_QUEUE_DESC desc{};desc.Type=D3D12_COMMAND_LIST_TYPE_DIRECT;
 ID3D12CommandQueue*q=nullptr;if(FAILED(d->CreateCommandQueue(&desc,IID_PPV_ARGS(&q))))return false;
 execute=reinterpret_cast<Execute>((*reinterpret_cast<void***>(q))[10]);
 reset=reinterpret_cast<Reset>((*reinterpret_cast<void***>(c))[10]);q->Release();
 device=d;d->AddRef();timing=wantTiming;csvPath=path;
 if(timing){
  D3D12_QUERY_HEAP_DESC query{};query.Type=D3D12_QUERY_HEAP_TYPE_TIMESTAMP;query.Count=Capacity*StampCount;
  if(FAILED(d->CreateQueryHeap(&query,IID_PPV_ARGS(&queries))))return false;
  D3D12_HEAP_PROPERTIES heap{};heap.Type=D3D12_HEAP_TYPE_READBACK;
  D3D12_RESOURCE_DESC r{};r.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;r.Width=Capacity*StampCount*sizeof(UINT64);r.Height=1;r.DepthOrArraySize=1;r.MipLevels=1;r.SampleDesc.Count=1;r.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  if(FAILED(d->CreateCommittedResource(&heap,D3D12_HEAP_FLAG_NONE,&r,D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&readback))))return false;
  if(FAILED(readback->Map(0,nullptr,reinterpret_cast<void**>(&mapped))))return false;
 }
 if(DetourAttach(reinterpret_cast<PVOID*>(&execute),OnExecute)!=NO_ERROR)return false;
 if(DetourAttach(reinterpret_cast<PVOID*>(&reset),OnReset)!=NO_ERROR){DetourDetach(reinterpret_cast<PVOID*>(&execute),OnExecute);return false;}
 return true;
}
static void Activate(){
 enabled=true;
 if(timing){FILE*f=nullptr;_wfopen_s(&f,csvPath.c_str(),L"w");if(f){fputs("evaluation,submission,width,height,scope,prepared,total_ms,encode_ms,guides_ms,model_ms,resolve_ms,post_ms,dropped,unavailable\n",f);fclose(f);}auto h=CreateThread(nullptr,0,Writer,nullptr,0,nullptr);if(h)CloseHandle(h);}
}
static void Mark(unsigned phase){
 if(!current||!timing||phase>=StampCount)return;
 current->list->EndQuery(queries,D3D12_QUERY_TYPE_TIMESTAMP,unsigned(current-slots.data())*StampCount+phase);current->mask|=1u<<phase;
}
static bool Begin(ID3D12GraphicsCommandList*c,unsigned w,unsigned h,bool outer){
 if(!enabled||!c||current)return false;
 if(c->GetType()!=D3D12_COMMAND_LIST_TYPE_DIRECT&&c->GetType()!=D3D12_COMMAND_LIST_TYPE_COMPUTE)return false;
 std::lock_guard<std::mutex> guard(mutex);CollectLocked();
 for(auto&s:slots)if(!s.list){s.list=c;s.id=++nextId;s.width=w;s.height=h;s.outer=outer;current=&s;Mark(0);return true;}
 ++unavailable;return false;
}
static void End(){
 if(!current)return;Mark(4);
 if(timing){unsigned index=unsigned(current-slots.data())*StampCount;
  // Resolve only written queries; unresolved phase gaps stay explicitly absent.
  for(unsigned i=0;i<StampCount;++i)if(current->mask&(1u<<i))current->list->ResolveQueryData(queries,D3D12_QUERY_TYPE_TIMESTAMP,index+i,1,readback,(index+i)*sizeof(UINT64));
  current->resolved=true;
 }current=nullptr;
}
struct ModelScope {
 bool owned,active;
 ModelScope(ID3D12GraphicsCommandList*c,unsigned w,unsigned h):owned(Begin(c,w,h,false)),active(current&&current->list==c){if(active)Mark(2);}
 ~ModelScope(){if(active){Mark(3);if(owned)End();}}
};
static Slot* Prepared(ID3D12GraphicsCommandList*c){
 if(!current||current->list!=c||current->prepared)return nullptr;
 if(!current->weights){
  D3D12_HEAP_PROPERTIES heap{};heap.Type=D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_DESC r{};r.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;r.Width=41408;r.Height=1;r.DepthOrArraySize=1;r.MipLevels=1;r.SampleDesc.Count=1;r.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;r.Flags=D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  if(FAILED(device->CreateCommittedResource(&heap,D3D12_HEAP_FLAG_NONE,&r,D3D12_RESOURCE_STATE_UNORDERED_ACCESS,nullptr,IID_PPV_ARGS(&current->weights))))return nullptr;
 }current->prepared=true;return current;
}
}
