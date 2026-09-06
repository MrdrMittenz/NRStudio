#include <cuda_runtime.h>
#include <cuda_fp8.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include "fp8_exact.cuh"

static void check(cudaError_t result) {
    if (result != cudaSuccess) { fprintf(stderr, "%s\n", cudaGetErrorString(result)); exit(2); }
}
struct Counts { unsigned encode, decode, packEncode, packDecode, fused; };
__global__ void verify(Counts* counts) {
    const unsigned i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= 65536) return;
    __half_raw h; h.x = uint16_t(i);
    const auto expected = __nv_cvt_halfraw_to_fp8(h, __NV_SATFINITE, __NV_E4M3);
    if (nr_half_to_e4m3(h.x) != expected) atomicAdd(&counts->encode, 1);
    if (i < 256 && nr_e4m3_to_half(uint8_t(i)) != __nv_cvt_fp8_to_halfraw(uint8_t(i), __NV_E4M3).x)
        atomicAdd(&counts->decode, 1);
    // Cover every possible packed FP8 pair, including both NaNs and signed zeros.
    const auto decoded = __nv_cvt_fp8x2_to_halfraw2(uint16_t(i), __NV_E4M3);
    const unsigned expectedPair = unsigned(decoded.x) | (unsigned(decoded.y) << 16);
    if (nr_e4m3x2_to_half2(uint16_t(i)) != expectedPair) atomicAdd(&counts->packDecode, 1);
    // Each half pattern occurs in both lanes, with varying independent companions.
    for (unsigned pattern = 0; pattern < 4; ++pattern) {
        const uint16_t other = uint16_t(pattern == 0 ? ~i : pattern == 1 ? i * 25173 + 13849 : pattern == 2 ? i ^ 0x8000 : 0x7e00);
        const __half2_raw pair{uint16_t(i), other};
        const auto pairExpected = __nv_cvt_halfraw2_to_fp8x2(pair, __NV_SATFINITE, __NV_E4M3);
        if (nr_half2_to_e4m3x2(i | (unsigned(other) << 16)) != pairExpected) atomicAdd(&counts->packEncode, 1);
        const auto roundtrip = __nv_cvt_fp8x2_to_halfraw2(pairExpected, __NV_E4M3);
        const unsigned expectedQuantized = unsigned(roundtrip.x) | (unsigned(roundtrip.y) << 16);
        if (nr_quantize_e4m3_half2(i | (unsigned(other) << 16)) != expectedQuantized) atomicAdd(&counts->fused, 1);
    }
}
int main() {
    unsigned hostEncode = 0, hostDecode = 0;
    for (unsigned i = 0; i < 65536; ++i) {
        __half_raw h; h.x = uint16_t(i);
        const auto expected = __nv_cvt_halfraw_to_fp8(h, __NV_SATFINITE, __NV_E4M3);
        if (nr_half_to_e4m3(h.x) != expected) {
            if (hostEncode < 5) printf("encode mismatch input=%04x got=%02x expected=%02x\n", i, nr_half_to_e4m3(h.x), expected);
            ++hostEncode;
        }
        if (i < 256 && nr_e4m3_to_half(uint8_t(i)) != __nv_cvt_fp8_to_halfraw(uint8_t(i), __NV_E4M3).x) {
            if (hostDecode < 5) printf("decode mismatch input=%02x got=%04x expected=%04x\n", i, nr_e4m3_to_half(uint8_t(i)), __nv_cvt_fp8_to_halfraw(uint8_t(i), __NV_E4M3).x);
            ++hostDecode;
        }
    }
    Counts* device; check(cudaMalloc(&device, sizeof(Counts))); check(cudaMemset(device, 0, sizeof(Counts)));
    verify<<<256,256>>>(device); check(cudaGetLastError()); check(cudaDeviceSynchronize());
    Counts result{}; check(cudaMemcpy(&result, device, sizeof(result), cudaMemcpyDeviceToHost)); check(cudaFree(device));
    printf("host_encode_mismatches=%u host_decode_mismatches=%u\n", hostEncode, hostDecode);
    printf("gpu_encode_mismatches=%u gpu_decode_mismatches=%u packed_encode_mismatches=%u packed_decode_mismatches=%u\n", result.encode, result.decode, result.packEncode, result.packDecode);
    printf("fused_quantizer_mismatches=%u tested_pairs=262144\n", result.fused);
    return hostEncode || hostDecode || result.encode || result.decode || result.packEncode || result.packDecode || result.fused ? 1 : 0;
}
