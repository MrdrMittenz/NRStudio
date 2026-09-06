#include <cuda.h>
#include <cuda_fp16.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

static void check(CUresult result) {
    if (result == CUDA_SUCCESS) return;
    const char* name = nullptr; cuGetErrorName(result, &name);
    fprintf(stderr, "CUDA error %d %s\n", result, name ? name : ""); exit(2);
}
template<class T> static void put(unsigned char* params, size_t offset, T value) {
    memcpy(params + offset, &value, sizeof(value));
}
int main(int argc, char** argv) {
    if (argc < 2) return 2;
    const bool small = argc > 2 && !strcmp(argv[2], "small");
    const bool tune = argc > 2 && !strcmp(argv[2], "cache");
    const unsigned width = small ? 256 : 2560, height = small ? 256 : 1440;
    const unsigned tensorHeight = small ? 256 : 1472;
    check(cuInit(0)); CUdevice device; CUcontext context;
    check(cuDeviceGet(&device, 0)); check(cuDevicePrimaryCtxRetain(&context, device)); check(cuCtxSetCurrent(context));
    CUmodule module; CUfunction kernel;
    check(cuModuleLoad(&module, argv[1]));
    check(cuModuleGetFunction(&kernel, module, "cc_tinlayout_fused_post_block_swin_1h_32_fp8"));
    size_t offset, size; check(cuFuncGetParamInfo(kernel, 0, &offset, &size));
    if (offset || (size != 184 && size != 192)) return 3;
    const bool prepared = size == 192;
    for (auto attribute : {CU_FUNC_ATTRIBUTE_NUM_REGS, CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES, CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES}) {
        int value; check(cuFuncGetAttribute(&value, attribute, kernel)); printf("attribute=%d value=%d\n", attribute, value);
    }
    CUstream stream; check(cuStreamCreate(&stream, CU_STREAM_NON_BLOCKING));
    // Overprovision synthetic tensor storage, including surrounding padding.
    // Exact native tensor extents/layout are not established by this harness.
    const size_t halos[] = {size_t(width) * 4 * 8, size_t(width) * 32 * 8, 0, 0};
    const size_t sizes[] = {size_t(tensorHeight) * width * 16 + 2 * halos[0], size_t(tensorHeight) * width * 32 + 2 * halos[1], 1024 * 1024, 4096};
    CUdeviceptr allocations[4], buffers[4]; constexpr size_t guard = 4096;
    for (int i = 0; i < 4; ++i) {
        check(cuMemAlloc(&allocations[i], sizes[i] + 2 * guard)); buffers[i] = allocations[i] + guard;
        check(cuMemsetD8Async(allocations[i], 0xa5, sizes[i] + 2 * guard, stream));
        check(cuMemsetD8Async(buffers[i], 0x08, sizes[i], stream));
    }
    check(cuMemsetD16Async(buffers[3], 0x3c00, sizes[3] / 2, stream));
    const int pattern = argc > 4 ? atoi(argv[4]) : 0;
    if (pattern < 0 || pattern > 5) return 9;
    if (pattern) {
        check(cuStreamSynchronize(stream));
        std::vector<unsigned short> host(sizes[0] / 2);
        const unsigned short values[] = {0xb400, 0xb000, 0x3000, 0x3400};
        for (size_t i = 0; i < host.size(); ++i) host[i] = values[(i * 13 + i / 31 + pattern) % 4];
        if (pattern <= 3) check(cuMemcpyHtoD(buffers[0], host.data(), sizes[0]));
        const unsigned char values8[] = {0x00,0x08,0x10,0x20,0x28,0x30,0x80,0x88,0x90,0xa0};
        for (int index : {1, 2}) {
            if (pattern == 3 || (pattern == 4 && index != 1) || (pattern == 5 && index != 2)) continue;
            std::vector<unsigned char> packed(sizes[index]);
            for (size_t i = 0; i < packed.size(); ++i) packed[i] = values8[(i * 7 + i / 19 + pattern * 3) % 10];
            check(cuMemcpyHtoD(buffers[index], packed.data(), sizes[index]));
        }
    }
    printf("pattern=%d\n", pattern);
    CUdeviceptr preparedWeights = 0;
    if (prepared) {
        check(cuStreamSynchronize(stream));
        std::vector<unsigned char> packed(sizes[2]);
        check(cuMemcpyDtoH(packed.data(), buffers[2], packed.size()));
        std::vector<unsigned short> expanded(packed.size());
        for (size_t i = 0; i < packed.size(); i += 4) {
            const int order[] = {0,2,1,3};
            for (int j = 0; j < 4; ++j) {
                unsigned v = packed[i + order[j]];
                __half_raw raw; raw.x = static_cast<unsigned short>(((v & 0x7f) << 7) | ((v & 0x80) << 8));
                __half result = __float2half_rn(__half2float(__half(raw)) * 256.0f);
                expanded[i+j] = static_cast<__half_raw>(result).x;
            }
        }
        check(cuMemAlloc(&preparedWeights, expanded.size() * sizeof(unsigned short)));
        check(cuMemcpyHtoD(preparedWeights, expanded.data(), expanded.size() * sizeof(unsigned short)));
        printf("prepared_weight_bytes=%zu preparation=once_before_timing\n", expanded.size()*sizeof(unsigned short));
    }
    CUDA_ARRAY3D_DESCRIPTOR desc{}; desc.Width = width; desc.Height = height;
    desc.Format = CU_AD_FORMAT_FLOAT; desc.NumChannels = 4; desc.Flags = CUDA_ARRAY3D_SURFACE_LDST;
    CUarray inputArray, outputArray;
    check(cuArray3DCreate(&inputArray, &desc)); check(cuArray3DCreate(&outputArray, &desc));
    std::vector<float> input(size_t(width) * height * 4), sentinel(input.size(), -1234.0f), prior;
    for (unsigned y = 0; y < height; ++y) for (unsigned x = 0; x < width; ++x) {
        auto p = (size_t(y) * width + x) * 4;
        input[p] = 0.1f + 0.4f * x / width; input[p + 1] = 0.2f + 0.3f * y / height;
        input[p + 2] = 0.25f; input[p + 3] = 1.0f;
    }
    CUDA_MEMCPY2D copy{}; copy.srcMemoryType = CU_MEMORYTYPE_HOST; copy.srcHost = input.data();
    copy.srcPitch = width * 16; copy.dstMemoryType = CU_MEMORYTYPE_ARRAY; copy.dstArray = inputArray;
    copy.WidthInBytes = width * 16; copy.Height = height; check(cuMemcpy2DAsync(&copy, stream));
    CUDA_RESOURCE_DESC resource{}; resource.resType = CU_RESOURCE_TYPE_ARRAY; resource.res.array.hArray = inputArray;
    CUDA_TEXTURE_DESC texture{}; texture.addressMode[0] = texture.addressMode[1] = CU_TR_ADDRESS_MODE_CLAMP;
    texture.filterMode = CU_TR_FILTER_MODE_LINEAR; texture.flags = CU_TRSF_NORMALIZED_COORDINATES;
    CUtexObject tex; check(cuTexObjectCreate(&tex, &resource, &texture, nullptr));
    resource.res.array.hArray = outputArray; CUsurfObject surface; check(cuSurfObjectCreate(&surface, &resource));
    alignas(8) unsigned char params[192]{};
    put(params, 0, buffers[0] + halos[0]); put(params, 8, buffers[1] + halos[1]); put(params, 16, surface); put(params, 24, buffers[2]);
    put(params, 32, int(tensorHeight)); put(params, 36, int(width)); put(params, 40, -4); put(params, 44, -4);
    put(params, 48, 0.03125f); put(params, 52, 1); put(params, 56, tex);
    put(params, 72, float(width)); put(params, 76, float(height)); put(params, 80, 1.0f / width); put(params, 84, 1.0f / height);
    put(params, 104, buffers[3]); put(params, 112, uint64_t(1));
    put(params, 164, 1.0f / width); put(params, 168, 1.0f / height); put(params, 172, int(width)); put(params, 176, int(height));
    if (prepared) put(params, 184, preparedWeights);
    void* args[] = {params}; CUevent start, end; check(cuEventCreate(&start, 0)); check(cuEventCreate(&end, 0));
    for (int run = 0; run < (tune ? 30 : 5); ++run) {
        const int mode = tune ? (run % 3 + run / 3) % 3 : 0;
        const int carveouts[] = {-1, 0, 100};
        check(cuFuncSetAttribute(kernel, CU_FUNC_ATTRIBUTE_PREFERRED_SHARED_MEMORY_CARVEOUT, carveouts[mode]));
        copy.srcHost = sentinel.data(); copy.dstArray = outputArray; check(cuMemcpy2DAsync(&copy, stream));
        check(cuEventRecord(start, stream));
        const int launches = tune ? 10 : 1;
        for (int launch = 0; launch < launches; ++launch)
            check(cuLaunchKernel(kernel, width / 8 + 1, tensorHeight / 8 + 1, 1, 32, 1, 1, 0, stream, args, nullptr));
        check(cuEventRecord(end, stream)); check(cuEventSynchronize(end));
        float ms; check(cuEventElapsedTime(&ms, start, end)); ms /= launches;
        std::vector<float> output(input.size()); CUDA_MEMCPY2D read{};
        read.srcMemoryType = CU_MEMORYTYPE_ARRAY; read.srcArray = outputArray; read.dstMemoryType = CU_MEMORYTYPE_HOST;
        read.dstHost = output.data(); read.dstPitch = width * 16; read.WidthInBytes = width * 16; read.Height = height;
        check(cuMemcpy2D(&read)); size_t unwritten = 0, invalid = 0; double sum = 0;
        for (auto value : output) { unwritten += value == -1234.0f; invalid += !std::isfinite(value); sum += value; }
        const bool exact = !run || !memcmp(output.data(), prior.data(), output.size() * sizeof(float));
        for (int i = 0; i < 4; ++i) {
            unsigned char edge[guard]; check(cuMemcpyDtoH(edge, allocations[i], guard));
            for (auto b : edge) if (b != 0xa5) return 4;
            check(cuMemcpyDtoH(edge, buffers[i] + sizes[i], guard)); for (auto b : edge) if (b != 0xa5) return 5;
        }
        printf("run=%d carveout=%d gpu_ms=%.6f launches=%d unwritten=%zu invalid=%zu sum=%.9g repeat=%s guards=pass\n", run, carveouts[mode], ms, launches, unwritten, invalid, sum, exact ? "pass" : "FAIL"); fflush(stdout);
        if (unwritten || invalid || !exact || sum == 0) return 6;
        if (argc > 3 && run == (tune ? 29 : 4)) {
            FILE* file = nullptr;
            if (fopen_s(&file, argv[3], "wb") || !file) return 7;
            const size_t written = fwrite(output.data(), sizeof(float), output.size(), file);
            fclose(file);
            if (written != output.size()) return 8;
        }
        prior.swap(output);
    }
    check(cuEventDestroy(start)); check(cuEventDestroy(end)); check(cuTexObjectDestroy(tex)); check(cuSurfObjectDestroy(surface));
    check(cuArrayDestroy(inputArray)); check(cuArrayDestroy(outputArray));
    for (auto p : allocations) check(cuMemFree(p));
    if (preparedWeights) check(cuMemFree(preparedWeights));
    check(cuStreamDestroy(stream)); check(cuModuleUnload(module)); check(cuDevicePrimaryCtxRelease(device)); return 0;
}
