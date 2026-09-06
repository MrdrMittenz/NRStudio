#include <cuda.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

static void check(CUresult result) {
    if (result == CUDA_SUCCESS) return;
    const char* name = nullptr;
    cuGetErrorName(result, &name);
    fprintf(stderr, "CUDA error %d %s\n", result, name ? name : "");
    exit(2);
}

// Layouts recovered from model dispatch bytes and PTX parameter accesses.
struct Ffwd {
    CUdeviceptr input, output, weights;
    int height, width;
    CUdeviceptr inputFlags;
    int offsetX, offsetY;
    CUdeviceptr outputFlags;
};
struct Projection {
    CUdeviceptr input, residual, output, weights;
    int height, width;
    CUdeviceptr unused, outputFlags, inputFlags;
    int offsetX, offsetY;
};
static_assert(sizeof(Ffwd) == 56);
static_assert(sizeof(Projection) == 72);

int main(int argc, char** argv) {
    if (argc < 2) return 2;
    // Optional exact kernel name selects a single case for application replay.
    const std::string selected = argc > 2 ? argv[2] : "";
    check(cuInit(0));
    CUdevice device;
    CUcontext context;
    check(cuDeviceGet(&device, 0));
    check(cuDevicePrimaryCtxRetain(&context, device));
    check(cuCtxSetCurrent(context));
    CUmodule module;
    check(cuModuleLoad(&module, argv[1]));
    CUstream stream;
    check(cuStreamCreate(&stream, CU_STREAM_NON_BLOCKING));
    CUevent start, end;
    check(cuEventCreate(&start, 0));
    check(cuEventCreate(&end, 0));
    constexpr size_t body = 16 * 1024 * 1024, guard = 4096;
    CUdeviceptr allocations[6], buffers[6];
    for (int i = 0; i < 6; ++i) {
        check(cuMemAlloc(&allocations[i], body + 2 * guard));
        buffers[i] = allocations[i] + guard;
        check(cuMemsetD8Async(allocations[i], 0xa5, body + 2 * guard, stream));
    }
    int cases = 0;
    for (int projection = 0; projection < 2; ++projection) {
        std::vector<unsigned char> reference;
        for (int chained = 0; chained < 2; ++chained) {
            std::string name = projection ? "cc_split_swin_16h_ffwd_proj_512" : "cc_split_swin_16h_ffwd_512";
            name += chained ? "_chained_fp8" : "_fp8";
            if (!selected.empty() && selected != name) continue;
            ++cases;
            CUfunction function;
            check(cuModuleGetFunction(&function, module, name.c_str()));
            size_t offset, bytes;
            check(cuFuncGetParamInfo(function, 0, &offset, &bytes));
            if (offset || bytes != (projection ? sizeof(Projection) : sizeof(Ffwd))) return 3;
            int registers, local, shared;
            check(cuFuncGetAttribute(&registers, CU_FUNC_ATTRIBUTE_NUM_REGS, function));
            check(cuFuncGetAttribute(&local, CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES, function));
            check(cuFuncGetAttribute(&shared, CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES, function));
            printf("kernel=%s registers=%d local_bytes=%d shared_bytes=%d\n", name.c_str(), registers, local, shared);
            for (int seed = 0; seed < 2; ++seed) {
                std::vector<unsigned char> prior;
                for (int run = 0; run < 5; ++run) {
                    // Small finite bit patterns for synthetic packed data and weights.
                    // These are not captured model activations or trained weights.
                    check(cuMemsetD8Async(buffers[0], seed ? 0x09 : 0x08, body, stream));
                    check(cuMemsetD8Async(buffers[1], 0x08, body, stream));
                    check(cuMemsetD8Async(buffers[2], 0xcd, body, stream));
                    check(cuMemsetD8Async(buffers[3], seed ? 0x06 : 0x08, body, stream));
                    // Inputs are already populated in this stream. Zero denotes ready.
                    // Output flags start at -1 and must be published by the kernel.
                    check(cuMemsetD32Async(buffers[4], 0, body / 4, stream));
                    check(cuMemsetD32Async(buffers[5], ~0u, body / 4, stream));
                    Ffwd ff{buffers[0], buffers[2], buffers[3], 48, 80, buffers[4], 0, 0, buffers[5]};
                    Projection proj{buffers[0], buffers[1], buffers[2], buffers[3], 48, 80, 0, buffers[5], buffers[4], 0, 0};
                    void* args[] = {projection ? static_cast<void*>(&proj) : static_cast<void*>(&ff)};
                    check(cuEventRecord(start, stream));
                    check(cuLaunchKernel(function, projection ? 20 : 10, 6, projection ? 1 : 2,
                                         32, projection ? 4 : 8, 1, 0, stream, args, nullptr));
                    check(cuEventRecord(end, stream));
                    check(cuEventSynchronize(end));
                    float ms;
                    check(cuEventElapsedTime(&ms, start, end));
                    std::vector<unsigned char> output(body);
                    check(cuMemcpyDtoH(output.data(), buffers[2], body));
                    const size_t changed = std::count_if(output.begin(), output.end(), [](unsigned char b) { return b != 0xcd; });
                    if (!changed || (run && output != prior)) return 4;
                    // Retain both seeds for exact plain/chained comparison.
                    if (!chained && run == 0) reference.insert(reference.end(), output.begin(), output.end());
                    if (chained && !reference.empty() && !std::equal(output.begin(), output.end(), reference.begin() + seed * body)) return 5;
                    size_t published = 0;
                    if (chained) {
                        std::vector<unsigned> flags(body / 4);
                        check(cuMemcpyDtoH(flags.data(), buffers[5], body));
                        for (auto flag : flags) {
                            if (flag == 0) ++published;
                            else if (flag != ~0u) return 9;
                        }
                        if (!published) return 10;
                    }
                    for (int i = 0; i < 6; ++i) {
                        unsigned char edge[guard];
                        check(cuMemcpyDtoH(edge, allocations[i], guard));
                        for (auto b : edge) if (b != 0xa5) return 6;
                        check(cuMemcpyDtoH(edge, buffers[i] + body, guard));
                        for (auto b : edge) if (b != 0xa5) return 7;
                    }
                    printf("seed=%d run=%d gpu_us=%.3f changed_bytes=%zu published_flags=%zu repeat=pass guards=pass comparison=%s\n",
                           seed, run, ms * 1000, changed, published, chained && !reference.empty() ? "bit_exact_plain" : "not_compared");
                    fflush(stdout);
                    prior.swap(output);
                }
            }
        }
    }
    for (auto allocation : allocations) check(cuMemFree(allocation));
    check(cuEventDestroy(start));
    check(cuEventDestroy(end));
    check(cuStreamDestroy(stream));
    check(cuModuleUnload(module));
    check(cuDevicePrimaryCtxRelease(device));
    return cases ? 0 : 8;
}
