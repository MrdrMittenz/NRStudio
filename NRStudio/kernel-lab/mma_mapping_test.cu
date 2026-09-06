#include <cuda_runtime.h>
#include <cuda_fp8.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>
#include "fp8_mma.cuh"

static void check(cudaError_t result) {
    if (result != cudaSuccess) { fprintf(stderr, "%s\n", cudaGetErrorString(result)); exit(2); }
}

__global__ void matrix_test(const unsigned char* inputA, const unsigned char* inputB,
                            const unsigned short* inputC, unsigned short* output) {
    const unsigned test = blockIdx.x, lane = threadIdx.x;
    const unsigned group = lane / 4, thread = lane % 4;
    unsigned a[4]{}, b[2]{}, c[2], d[2];
    #pragma unroll
    for (int i = 0; i < 16; ++i) {
        unsigned row = group + ((i / 4) % 2) * 8;
        unsigned column = thread * 4 + (i % 4) + (i / 8) * 16;
        a[i / 4] |= unsigned(inputA[test * 512 + row * 32 + column]) << ((i % 4) * 8);
    }
    #pragma unroll
    for (int i = 0; i < 8; ++i) {
        unsigned row = thread * 4 + (i % 4) + (i / 4) * 16;
        b[i / 4] |= unsigned(inputB[test * 256 + row * 8 + group]) << ((i % 4) * 8);
    }
    #pragma unroll
    for (int i = 0; i < 2; ++i) {
        unsigned index = test * 128 + (group + i * 8) * 8 + thread * 2;
        c[i] = unsigned(inputC[index]) | (unsigned(inputC[index + 1]) << 16);
    }
    nr_mma_e4m3_m16n8k32(a, b, c, d);
    #pragma unroll
    for (int i = 0; i < 2; ++i) {
        unsigned index = test * 128 + (group + i * 8) * 8 + thread * 2;
        output[index] = uint16_t(d[i]); output[index + 1] = uint16_t(d[i] >> 16);
    }
}

int main() {
    constexpr unsigned tests = 128;
    std::vector<unsigned char> a(tests * 512), b(tests * 256);
    std::vector<unsigned short> c(tests * 128), output(c.size());
    std::vector<float> af(a.size()), bf(b.size()), cf(c.size());
    const float values[] = {-2, -1, -0.5f, 0, 0.5f, 1, 2};
    for (unsigned t = 0; t < tests; ++t) {
        for (unsigned row = 0; row < 16; ++row) for (unsigned k = 0; k < 32; ++k) {
            // First 32 cases route selected A values through a sparse B.
            // Next 32 route selected B rows through sparse A; remaining cases are dense.
            float value = t >= 32 && t < 64 ? (k == (row + t) % 32 ? 1.f : 0.f) : values[(row * 11 + k * 3 + t) % 7];
            unsigned index = t * 512 + row * 32 + k;
            af[index] = value; a[index] = __nv_cvt_float_to_fp8(value, __NV_SATFINITE, __NV_E4M3);
        }
        for (unsigned k = 0; k < 32; ++k) for (unsigned column = 0; column < 8; ++column) {
            float value = t < 32 ? (k == (column + t) % 32 ? 1.f : 0.f) : values[(k * 5 + column * 2 + t) % 7];
            unsigned index = t * 256 + k * 8 + column;
            bf[index] = value; b[index] = __nv_cvt_float_to_fp8(value, __NV_SATFINITE, __NV_E4M3);
        }
        for (unsigned i = 0; i < 128; ++i) {
            float value = (int((t + i) % 5) - 2) * 0.25f;
            cf[t * 128 + i] = value;
            __half_raw raw = __float2half(value); c[t * 128 + i] = raw.x;
        }
    }
    unsigned char *da, *db; unsigned short *dc, *dd;
    check(cudaMalloc(&da, a.size())); check(cudaMalloc(&db, b.size()));
    check(cudaMalloc(&dc, c.size() * 2)); check(cudaMalloc(&dd, output.size() * 2));
    check(cudaMemcpy(da, a.data(), a.size(), cudaMemcpyHostToDevice)); check(cudaMemcpy(db, b.data(), b.size(), cudaMemcpyHostToDevice));
    check(cudaMemcpy(dc, c.data(), c.size() * 2, cudaMemcpyHostToDevice)); check(cudaMemset(dd, 0xcd, output.size() * 2));
    matrix_test<<<tests,32>>>(da, db, dc, dd); check(cudaGetLastError()); check(cudaDeviceSynchronize());
    check(cudaMemcpy(output.data(), dd, output.size() * 2, cudaMemcpyDeviceToHost));
    unsigned mismatches = 0;
    for (unsigned t = 0; t < tests; ++t) for (unsigned row = 0; row < 16; ++row) for (unsigned column = 0; column < 8; ++column) {
        unsigned index = t * 128 + row * 8 + column;
        // Quarter-integer products and sums in this suite are exactly representable.
        float expected = cf[index];
        for (unsigned k = 0; k < 32; ++k) expected += af[t * 512 + row * 32 + k] * bf[t * 256 + k * 8 + column];
        float actual = __half2float(__half(__half_raw{output[index]}));
        if (actual != expected) {
            if (mismatches < 5) printf("test=%u row=%u col=%u actual=%g expected=%g\n", t, row, column, actual, expected);
            ++mismatches;
        }
    }
    printf("matrices=%u compared_values=%zu mismatches=%u\n", tests, output.size(), mismatches);
    check(cudaFree(da)); check(cudaFree(db)); check(cudaFree(dc)); check(cudaFree(dd));
    return mismatches ? 1 : 0;
}
