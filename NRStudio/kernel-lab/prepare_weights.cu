#include <cuda_runtime.h>
struct Params { const unsigned* source; unsigned* output; unsigned words; };
extern "C" __global__ void nrPrepareWeights(Params p) {
    unsigned i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= p.words) return;
    unsigned word = p.source[i];
    #pragma unroll
    for (unsigned part=0;part<2;++part) {
        unsigned shifted=word>>(part*8);
        unsigned raw=((shifted&0x007f007fu)<<7)|((shifted&0x00800080u)<<8);
        unsigned result, scale=0x5c005c00;
        asm("mul.f16x2 %0, %1, %2;" : "=r"(result) : "r"(raw),"r"(scale));
        p.output[i*2+part]=result;
    }
}
