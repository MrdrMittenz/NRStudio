#include <cuda.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
static void ck(CUresult x){if(x){const char* s=nullptr;cuGetErrorName(x,&s);fprintf(stderr,"CUDA %d %s\n",x,s?s:"");exit(2);}}
template<class T> void put(unsigned char* p,int o,T v){memcpy(p+o,&v,sizeof(v));}
int main(int argc,char** argv){
 if(argc<3)return 1;
 ck(cuInit(0));CUdevice d;CUcontext c;ck(cuDeviceGet(&d,0));ck(cuDevicePrimaryCtxRetain(&c,d));ck(cuCtxSetCurrent(c));
 CUmodule m;CUfunction f;ck(cuModuleLoad(&m,argv[1]));ck(cuModuleGetFunction(&f,m,"cc_tinlayout_fused_swin_4h_128_4_chained_fp8"));
 size_t off,sz;ck(cuFuncGetParamInfo(f,0,&off,&sz));if(off||sz!=88)return 3;
 for(auto a:{CU_FUNC_ATTRIBUTE_NUM_REGS,CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES,CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES}){int v;ck(cuFuncGetAttribute(&v,a,f));printf("attribute=%d value=%d\n",a,v);}
 // Match captured 1440p dispatch dimensions. Synthetic buffers, not captured activations.
 // Large initialized halos keep the recovered blocked layout inside guarded allocations.
 constexpr size_t body=128*1024*1024,guard=4096,halo=1024*1024;
 CUdeviceptr alloc[5],b[5];std::vector<unsigned char> host(body);
 int seed=argc>3?atoi(argv[3]):1;
 unsigned rng=0x9e3779b9u ^ static_cast<unsigned>(seed);
 for(int i=0;i<5;i++){
  ck(cuMemAlloc(&alloc[i],body+2*guard));b[i]=alloc[i]+guard;
  ck(cuMemsetD8(alloc[i],0xa5,body+2*guard));
  for(size_t j=0;j<body;j++){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;host[j]=(i==3?0:i==4?0xff:i==1?0xcd:16+((rng>>24)%5)*8 + ((rng & 1u)?128:0));}
  ck(cuMemcpyHtoD(b[i],host.data(),body));
 }
 alignas(8) unsigned char p[88]{};
 put(p,0,b[0]+halo);put(p,8,b[1]+halo);put(p,16,b[2]);put(p,32,184);put(p,36,320);put(p,40,-4);put(p,44,-4);put(p,48,b[3]);put(p,64,b[4]);
 void* args[]={p};CUevent a,z;ck(cuEventCreate(&a,0));ck(cuEventCreate(&z,0));
 for(int r=0;r<100;r++)ck(cuLaunchKernel(f,41,24,1,32,4,1,0,0,args,nullptr));ck(cuCtxSynchronize());
 for(int r=0;r<8;r++){
  ck(cuEventRecord(a,0));ck(cuLaunchKernel(f,41,24,1,32,4,1,0,0,args,nullptr));ck(cuEventRecord(z,0));ck(cuEventSynchronize(z));float ms;ck(cuEventElapsedTime(&ms,a,z));printf("run=%d gpu_ms=%.6f\n",r,ms);
 }
 std::vector<unsigned char> edge(guard);
 for(int i=0;i<5;i++)for(int end=0;end<2;end++){
  ck(cuMemcpyDtoH(edge.data(),alloc[i]+(end?guard+body:0),guard));for(auto v:edge)if(v!=0xa5){fprintf(stderr,"guard failure %d %d\n",i,end);return 4;}
 }
 ck(cuMemcpyDtoH(host.data(),b[1],body));FILE* out=nullptr;if(fopen_s(&out,argv[2],"wb")||!out)return 5;fwrite(host.data(),1,host.size(),out);fclose(out);
 std::vector<int> flags(41*24);ck(cuMemcpyDtoH(flags.data(),b[4],flags.size()*4));for(int v:flags)if(v!=0)return 6;
 puts("guards and completion flags PASS");for(auto x:alloc)ck(cuMemFree(x));ck(cuModuleUnload(m));ck(cuDevicePrimaryCtxRelease(d));return 0;
}
