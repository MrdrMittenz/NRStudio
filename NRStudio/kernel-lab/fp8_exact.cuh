#pragma once
#include <stdint.h>
#include <cuda_fp16.h>

// E4M3 finite saturation, round-to-nearest-even. Input is binary16 bits.
// All 65,536 input patterns are checked against the CUDA header implementation.
__host__ __device__ inline uint8_t nr_half_to_e4m3(uint16_t bits) {
    const unsigned magnitude = bits & 0x7fff;
    const unsigned sign = (bits >> 8) & 0x80;
    if (magnitude > 0x7c00) return 0x7f; // Canonical NaN, discard sign.
    if (magnitude >= 0x5f00) return uint8_t(sign | 0x7e);
    if (magnitude <= 0x1400) return uint8_t(sign); // <= half a minimum FP8 subnormal.
    unsigned result;
    if (magnitude >= 0x2400) {
        // Binary16 has seven more fraction bits and an exponent bias eight higher.
        result = ((magnitude + 63 + ((magnitude >> 7) & 1)) >> 7) - 64;
    } else {
        const unsigned exponent = magnitude >> 10;
        const unsigned significand = (magnitude & 1023) | 1024;
        const unsigned shift = 16 - exponent;
        result = (significand + ((1u << (shift - 1)) - 1) + ((significand >> shift) & 1)) >> shift;
    }
    return uint8_t(sign | result);
}

// E4M3 bits to binary16, including signed zero and the CUDA canonical NaN.
__host__ __device__ inline uint16_t nr_e4m3_to_half(uint8_t bits) {
    const unsigned magnitude = bits & 0x7f;
    const unsigned sign = (unsigned(bits) & 0x80) << 8;
    if (magnitude == 0x7f) return 0x7fff;
    if (magnitude >= 8) return uint16_t(sign | ((magnitude << 7) + 0x2000));
    if (!magnitude) return uint16_t(sign);
    // Only seven subnormal magnitudes; fixed comparisons avoid a normalization loop.
    const unsigned exponent = magnitude >= 4 ? 8 : magnitude >= 2 ? 7 : 6;
    const unsigned mantissa = (magnitude << (16 - exponent)) & 1023;
    return uint16_t(sign | (exponent << 10) | mantissa);
}

__host__ __device__ inline uint16_t nr_half2_to_e4m3x2(uint32_t bits) {
    return uint16_t(nr_half_to_e4m3(uint16_t(bits)) |
                    (unsigned(nr_half_to_e4m3(uint16_t(bits >> 16))) << 8));
}

__host__ __device__ inline uint32_t nr_e4m3x2_to_half2(uint16_t bits) {
    return unsigned(nr_e4m3_to_half(uint8_t(bits))) |
           (unsigned(nr_e4m3_to_half(uint8_t(bits >> 8))) << 16);
}

__host__ __device__ inline __half2 nr_as_half2(uint32_t bits) {
    return __half2(__half2_raw{uint16_t(bits), uint16_t(bits >> 16)});
}
__host__ __device__ inline uint32_t nr_as_bits(__half2 value) {
    const __half2_raw raw = value;
    return unsigned(raw.x) | (unsigned(raw.y) << 16);
}

// Keep values in binary16 while performing exactly the E4M3 encode/decode
// quantization. Intended for a future fused arithmetic path, not FP8 storage.
__host__ __device__ inline uint32_t nr_quantize_e4m3_half2(uint32_t bits) {
    const unsigned sign = bits & 0x80008000u;
    const __half2 magnitude = nr_as_half2(bits & 0x7fff7fffu);
    const unsigned nanMask = __hneu2_mask(magnitude, magnitude);
    const __half2 clamped = __hmin2(magnitude, __float2half2_rn(448.0f));
    const unsigned bounded = nr_as_bits(clamped);
    // No cross-lane carry is possible: each bounded magnitude is <= 0x5f00.
    const unsigned normal = (bounded + 0x003f003fu + ((bounded >> 7) & 0x00010001u)) & 0xff80ff80u;
    const __half2 two = __float2half2_rn(2.0f);
    const unsigned subnormal = nr_as_bits(__hsub2(__hadd2(clamped, two), two));
    const unsigned normalMask = __hge2_mask(clamped, __float2half2_rn(0.015625f));
    const unsigned quantized = ((normal & normalMask) | (subnormal & ~normalMask)) | sign;
    return (quantized & ~nanMask) | (0x7fff7fffu & nanMask);
}
