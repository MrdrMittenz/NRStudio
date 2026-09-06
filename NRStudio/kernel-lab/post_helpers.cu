#include <cuda_runtime.h>
#include "fp8_mma.cuh"
extern "C" __device__ unsigned nrEncode(unsigned value) {
#ifdef NR_FAST_FP8
    return nr_pack_half2_e4m3(value);
#else
    return nr_half2_to_e4m3x2(value);
#endif
}
extern "C" __device__ unsigned nrDecode(unsigned value) { return NR_MMA_DECODE(uint16_t(value)); }
extern "C" __device__ unsigned nrQuantize(unsigned value) { return nr_quantize_e4m3_mma_half2(value); }
extern "C" __device__ unsigned long long nrMma(unsigned a0, unsigned a1, unsigned a2, unsigned a3,
                                              unsigned b0, unsigned b1, unsigned c0, unsigned c1) {
    unsigned a[4]{a0,a1,a2,a3}, b[2]{b0,b1}, c[2]{c0,c1}, d[2];
#ifdef NR_MMA_CONTIGUOUS
    nr_mma_e4m3_contiguous_m16n8k32(a,b,c,d);
#else
    nr_mma_e4m3_m16n8k32(a,b,c,d);
#endif
    return (static_cast<unsigned long long>(d[1]) << 32) | d[0];
}
