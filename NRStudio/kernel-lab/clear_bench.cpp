#include <cuda.h>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cstdlib>
void check(CUresult r){if(r!=CUDA_SUCCESS){const char*n=nullptr;cuGetErrorName(r,&n);printf("CUDA error %d %s\n",r,n?n:"");exit(2);}}
int main(int argc,char**argv){
 if(argc!=3)return 2;
 check(cuInit(0));CUdevice dev;check(cuDeviceGet(&dev,0));CUcontext ctx;check(cuDevicePrimaryCtxRetain(&ctx,dev));check(cuCtxSetCurrent(ctx));
 CUmodule mod[2];CUfunction fn[2];check(cuModuleLoad(&mod[0],argv[1]));check(cuModuleLoad(&mod[1],argv[2]));check(cuModuleGetFunction(&fn[0],mod[0],"cc_cb_clear"));check(cuModuleGetFunction(&fn[1],mod[1],"clear_vector"));
 size_t offset=0,paramSize=0;check(cuFuncGetParamInfo(fn[0],0,&offset,&paramSize));printf("original parameter 0 offset=%zu bytes=%zu\n",offset,paramSize);if(offset!=0||paramSize!=16)return 3;
 CUstream stream;check(cuStreamCreate(&stream,CU_STREAM_NON_BLOCKING));CUevent start,end;check(cuEventCreate(&start,0));check(cuEventCreate(&end,0));
 const int sizes[]={1,3,4,31,32,33,255,256,257,4096,57600,3686400};
 for(int count:sizes){
  CUdeviceptr alloc;const int guard=32;size_t bytes=(size_t(count)+2*guard)*4;check(cuMemAlloc(&alloc,bytes));CUdeviceptr output=alloc+guard*4;void*params[]={&output,&count};struct Packed{CUdeviceptr output;int count;unsigned padding;}packed{output,count,0};static_assert(sizeof(Packed)==16);void*originalParams[]={&packed};
  for(int variant=0;variant<3;variant++){
   check(cuMemsetD8Async(alloc,0xa5,bytes,stream));
   if(variant==2)check(cuMemsetD32Async(output,~0u,count,stream));
   else check(cuLaunchKernel(fn[variant],(count+(variant?1023:255))/(variant?1024:256),1,1,256,1,1,0,stream,variant?params:originalParams,nullptr));
   check(cuStreamSynchronize(stream));std::vector<unsigned> values(count+2*guard);check(cuMemcpyDtoH(values.data(),alloc,bytes));
   for(size_t i=0;i<values.size();i++){unsigned expected=i>=guard&&i<size_t(count+guard)?~0u:0xa5a5a5a5;if(values[i]!=expected){printf("FAIL variant=%d n=%d offset=%zu\n",variant,count,i);return 4;}}
  }
  printf("PASS n=%d bit-exact output and prefix/suffix guards\n",count);
  for(int run=0;run<4;run++)for(int order=0;order<3;order++){
   int variant=(run+order)%3;int repeats=count>100000?100:1000;
   // Warm each path before measuring; rotate order to limit systematic drift.
   auto launch=[&](){if(variant==2)check(cuMemsetD32Async(output,~0u,count,stream));else check(cuLaunchKernel(fn[variant],(count+(variant?1023:255))/(variant?1024:256),1,1,256,1,1,0,stream,variant?params:originalParams,nullptr));};
   for(int j=0;j<10;j++)launch();check(cuEventRecord(start,stream));for(int j=0;j<repeats;j++)launch();check(cuEventRecord(end,stream));check(cuEventSynchronize(end));float ms;check(cuEventElapsedTime(&ms,start,end));printf("timing n=%d run=%d variant=%d mean_us=%.6f\n",count,run,variant,ms*1000/repeats);
  }
  check(cuMemFree(alloc));
 }
 check(cuModuleUnload(mod[1]));check(cuModuleUnload(mod[0]));check(cuStreamDestroy(stream));check(cuDevicePrimaryCtxRelease(dev));
 return 0;
}
