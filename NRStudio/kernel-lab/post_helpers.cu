#include <cuda_runtime.h>
#include "fp8_mma.cuh"
extern "C" __device__ unsigned nrEncode(unsigned value) { return nr_half2_to_e4m3x2(value); }
extern "C" __device__ unsigned nrDecode(unsigned value) { return nr_e4m3x2_to_half2(uint16_t(value)); }
extern "C" __device__ unsigned long long nrMma(unsigned a0, unsigned a1, unsigned a2, unsigned a3,
                                              unsigned b0, unsigned b1, unsigned c0, unsigned c1) {
    unsigned a[4]{a0,a1,a2,a3}, b[2]{b0,b1}, c[2]{c0,c1}, d[2];
    nr_mma_e4m3_m16n8k32(a,b,c,d);
    return (static_cast<unsigned long long>(d[1]) << 32) | d[0];
}
