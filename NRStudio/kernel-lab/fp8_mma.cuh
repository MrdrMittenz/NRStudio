#pragma once
#include "fp8_exact.cuh"
#ifndef NR_MMA_REVERSE
#define NR_MMA_REVERSE 0
#endif
#ifdef NR_FAST_FP8
#define NR_MMA_DECODE nr_unpack_e4m3_half2
#else
#define NR_MMA_DECODE nr_e4m3x2_to_half2
#endif

// E4M3 m16n8k32 row/column fragments -> two Ampere FP16 m16n8k16
// operations. This establishes operand mapping, not cross-architecture
// accumulation equivalence for arbitrary floating-point inputs.
// All 32 lanes must participate in every shuffle and MMA instruction.
__device__ __forceinline__ unsigned nr_gather_half_pair(unsigned word, unsigned source, unsigned lane) {
    const unsigned packed = __shfl_sync(0xffffffffu, word, source);
    return NR_MMA_DECODE(uint16_t(packed >> ((lane & 1) * 16)));
}

__device__ __forceinline__ void nr_mma_e4m3_contiguous_m16n8k32(
    const unsigned (&a)[4], const unsigned (&b)[2], const unsigned (&c)[2], unsigned (&d)[2]) {
    const unsigned lane = threadIdx.x & 31;
    const unsigned source = (lane & ~3u) | ((lane & 3u) >> 1);
    unsigned c0 = c[0], c1 = c[1];
    #pragma unroll
    for (unsigned step = 0; step < 2; ++step) {
        const unsigned part = step ^ NR_MMA_REVERSE;
        const unsigned a0 = nr_gather_half_pair(a[part * 2], source, lane);
        const unsigned a1 = nr_gather_half_pair(a[part * 2 + 1], source, lane);
        const unsigned a2 = nr_gather_half_pair(a[part * 2], source + 2, lane);
        const unsigned a3 = nr_gather_half_pair(a[part * 2 + 1], source + 2, lane);
        const unsigned b0 = nr_gather_half_pair(b[part], source, lane);
        const unsigned b1 = nr_gather_half_pair(b[part], source + 2, lane);
        unsigned r0, r1;
        asm volatile("mma.sync.aligned.m16n8k16.row.col.f16.f16.f16.f16 "
                     "{%0,%1}, {%2,%3,%4,%5}, {%6,%7}, {%8,%9};"
                     : "=r"(r0), "=r"(r1)
                     : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(b0), "r"(b1), "r"(c0), "r"(c1));
        c0 = r0; c1 = r1;
    }
    d[0] = c0; d[1] = c1;
}

// Pair-interleaved K partition. Apply the same K permutation to A and B,
// retaining each lane's data and avoiding the contiguous partition's shuffles.
// The real-valued dot product is unchanged, but FP16 accumulation order differs.
__device__ __forceinline__ void nr_mma_e4m3_m16n8k32(
    const unsigned (&a)[4], const unsigned (&b)[2], const unsigned (&c)[2], unsigned (&d)[2]) {
    unsigned c0 = c[0], c1 = c[1];
    #pragma unroll
    for (unsigned step = 0; step < 2; ++step) {
        const unsigned part = step ^ NR_MMA_REVERSE;
        const unsigned a0 = NR_MMA_DECODE(uint16_t(a[0] >> (part * 16)));
        const unsigned a1 = NR_MMA_DECODE(uint16_t(a[1] >> (part * 16)));
        const unsigned a2 = NR_MMA_DECODE(uint16_t(a[2] >> (part * 16)));
        const unsigned a3 = NR_MMA_DECODE(uint16_t(a[3] >> (part * 16)));
        const unsigned b0 = NR_MMA_DECODE(uint16_t(b[0] >> (part * 16)));
        const unsigned b1 = NR_MMA_DECODE(uint16_t(b[1] >> (part * 16)));
        unsigned r0, r1;
        asm volatile("mma.sync.aligned.m16n8k16.row.col.f16.f16.f16.f16 "
                     "{%0,%1}, {%2,%3,%4,%5}, {%6,%7}, {%8,%9};"
                     : "=r"(r0), "=r"(r1)
                     : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(b0), "r"(b1), "r"(c0), "r"(c1));
        c0 = r0; c1 = r1;
    }
    d[0] = c0; d[1] = c1;
}
