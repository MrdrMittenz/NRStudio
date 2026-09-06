#pragma once
#include "fp8_exact.cuh"
#ifndef NR_MMA_REVERSE
#define NR_MMA_REVERSE 0
#endif
#if defined(NR_MMA_WORD_PARTITION) && !defined(NR_MMA_STRIDED)
#error NR_MMA_WORD_PARTITION requires NR_MMA_STRIDED
#endif
#if defined(NR_MMA_RAW_DECODE) && !defined(NR_MMA_STRIDED)
#error NR_MMA_RAW_DECODE requires NR_MMA_STRIDED
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
__device__ __forceinline__ unsigned nr_decode_mma_word(unsigned word, unsigned part) {
#ifdef NR_MMA_RAW_DECODE
    // Match the mask/shift/half2-multiply expansion observed around the original
    // post-block MMA. FP8 NaN encodings become +/-480, not canonical half NaNs.
    // This experimental path is NOT the general-purpose FP8 decoder.
    const unsigned shifted = word >> (part * 8);
    const unsigned raw = ((shifted & 0x007f007fu) << 7) | ((shifted & 0x00800080u) << 8);
    return nr_as_bits(__hmul2(nr_as_half2(raw), __float2half2_rn(256.f)));
#elif defined(NR_MMA_STRIDED)
    const unsigned shifted = word >> (part * 8);
    return NR_MMA_DECODE(uint16_t((shifted & 0xffu) | ((shifted >> 8) & 0xff00u)));
#else
    return NR_MMA_DECODE(uint16_t(word >> (part * 16)));
#endif
}
__device__ __forceinline__ void nr_mma_e4m3_m16n8k32(
    const unsigned (&a)[4], const unsigned (&b)[2], const unsigned (&c)[2], unsigned (&d)[2]) {
    unsigned c0 = c[0], c1 = c[1];
    #pragma unroll
    for (unsigned step = 0; step < 2; ++step) {
        const unsigned part = step ^ NR_MMA_REVERSE;
#ifdef NR_MMA_WORD_PARTITION
        // Observed in the original post-block SASS: first two A words/first
        // B word, then the remaining words; even/odd bytes form each half2.
        const unsigned a0 = nr_decode_mma_word(a[part * 2], 0);
        const unsigned a1 = nr_decode_mma_word(a[part * 2 + 1], 0);
        const unsigned a2 = nr_decode_mma_word(a[part * 2], 1);
        const unsigned a3 = nr_decode_mma_word(a[part * 2 + 1], 1);
        const unsigned b0 = nr_decode_mma_word(b[part], 0);
        const unsigned b1 = nr_decode_mma_word(b[part], 1);
#else
        const unsigned a0 = nr_decode_mma_word(a[0], part);
        const unsigned a1 = nr_decode_mma_word(a[1], part);
        const unsigned a2 = nr_decode_mma_word(a[2], part);
        const unsigned a3 = nr_decode_mma_word(a[3], part);
        const unsigned b0 = nr_decode_mma_word(b[0], part);
        const unsigned b1 = nr_decode_mma_word(b[1], part);
#endif
        unsigned r0, r1;
        asm volatile("mma.sync.aligned.m16n8k16.row.col.f16.f16.f16.f16 "
                     "{%0,%1}, {%2,%3,%4,%5}, {%6,%7}, {%8,%9};"
                     : "=r"(r0), "=r"(r1)
                     : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(b0), "r"(b1), "r"(c0), "r"(c1));
        c0 = r0; c1 = r1;
    }
    d[0] = c0; d[1] = c1;
}
