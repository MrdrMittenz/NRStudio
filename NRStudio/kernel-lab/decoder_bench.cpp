#include <cuda.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdint>
void check(CUresult r){if(r!=CUDA_SUCCESS){const char*n=nullptr;cuGetErrorName(r,&n);printf("CUDA error %d %s\n",r,n?n:"");exit(2);}}
float half(uint16_t v){int e=(v>>10)&31;float m=float(v&1023);float f=e==31?(m?NAN:INFINITY):e?std::ldexp(1.f+m/1024.f,e-15):std::ldexp(m,-24);return v&32768?-f:f;}
struct Params{CUdeviceptr input,skip,output,partialHalf,stageFlags,inputFlags,partialFp8,weights;int inputH,inputW,outputH,outputW;};
static_assert(sizeof(Params)==80);
int main(int argc,char**argv){
 if(argc<2)return 2;check(cuInit(0));CUdevice dev;check(cuDeviceGet(&dev,0));CUcontext ctx;check(cuDevicePrimaryCtxRetain(&ctx,dev));check(cuCtxSetCurrent(ctx));
 CUmodule module;check(cuModuleLoad(&module,argv[1]));CUfunction kernel;check(cuModuleGetFunction(&kernel,module,"cc_dec_input_upsample_1024_512_fp8"));size_t offset,size;check(cuFuncGetParamInfo(kernel,0,&offset,&size));if(offset||size!=80)return 3;
 const bool large=argc>2&&strcmp(argv[2],"1440")==0;
 CUstream stream;check(cuStreamCreate(&stream,CU_STREAM_NON_BLOCKING));CUdeviceptr allocation[8],address[8];const size_t body=(large?16:2)*1024*1024,guard=4096,total=body+2*guard;
 for(int i=0;i<8;i++){check(cuMemAlloc(&allocation[i],total));address[i]=allocation[i]+guard;check(cuMemsetD8Async(allocation[i],0xa5,total,stream));check(cuMemsetD8Async(address[i],0,body,stream));}
 // Source/skip packed half values and FP8 weights. These are synthetic inputs,
 // not snapshots of a game. Address roles are inferred from PTX accesses.
 std::vector<unsigned short> input(body/2),skip(body/2);
 for(size_t i=0;i<input.size();i++){input[i]=0x2800+unsigned(i%17)*16;skip[i]=0x2400+unsigned(i%13)*16;}
 check(cuMemcpyHtoDAsync(address[0],input.data(),body,stream));check(cuMemcpyHtoDAsync(address[1],skip.data(),body,stream));check(cuMemsetD8Async(address[7],0x08,body,stream));
 Params params{address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7],large?24:8,large?40:8,large?48:12,large?80:12};void*args[]={&params};
 CUevent start,end;check(cuEventCreate(&start,0));check(cuEventCreate(&end,0));std::vector<unsigned short> prior;
 for(int run=0;run<3;run++){
  check(cuMemsetD16Async(address[2],0x7e00,body/2,stream));check(cuMemsetD8Async(address[3],0,body,stream));check(cuMemsetD8Async(address[6],0,body,stream));
  // z partitions publish increasing stage numbers; -1 means no partition done.
  check(cuMemsetD32Async(address[4],~0u,body/4,stream));check(cuMemsetD32Async(address[5],~0u,body/4,stream));
  check(cuEventRecord(start,stream));check(cuLaunchKernel(kernel,large?20:4,large?6:2,4,32,2,1,0,stream,args,nullptr));check(cuEventRecord(end,stream));check(cuEventSynchronize(end));float ms;check(cuEventElapsedTime(&ms,start,end));
  std::vector<unsigned short> output(body/2);check(cuMemcpyDtoH(output.data(),address[2],body));size_t written=0,invalid=0,changed=0;double sum=0;
  for(size_t i=0;i<output.size();i++){if(output[i]==0x7e00)continue;written++;float v=half(output[i]);invalid+=!std::isfinite(v);sum+=std::isfinite(v)?v:0;if(run&&output[i]!=prior[i])changed++;}
  for(int i=0;i<8;i++){std::vector<unsigned char> prefix(guard),suffix(guard);check(cuMemcpyDtoH(prefix.data(),allocation[i],guard));check(cuMemcpyDtoH(suffix.data(),address[i]+body,guard));for(auto v:prefix)if(v!=0xa5)return 5;for(auto v:suffix)if(v!=0xa5)return 6;}
  printf("run=%d gpu_us=%.6f written_half_values=%zu invalid=%zu changed_vs_previous=%zu sum=%.9g guards=pass\n",run,ms*1000,written,invalid,changed,sum);fflush(stdout);
  if(!written||invalid||changed||sum==0)return 7;prior=output;
 }
 for(auto p:allocation)check(cuMemFree(p));check(cuModuleUnload(module));check(cuStreamDestroy(stream));check(cuDevicePrimaryCtxRelease(dev));return 0;
}
