#include <cuda_runtime.h>
extern "C" __global__ void clear_vector(unsigned* output, int count) {
    unsigned i=(blockIdx.x*blockDim.x+threadIdx.x)*4;
    if(i+3<(unsigned)count) {
        reinterpret_cast<uint4*>(output)[i/4]=make_uint4(~0u,~0u,~0u,~0u);
    } else {
        for(unsigned j=i;j<(unsigned)count;j++)output[j]=~0u;
    }
}
