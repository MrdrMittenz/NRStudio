#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <cstdint>

namespace ChainTiming {
static constexpr unsigned capacity = 32768;
static ID3D12QueryHeap* heap = nullptr;
static ID3D12Resource* readback = nullptr;
static UINT64 frequency = 0;
static bool completed = false;
struct Row { unsigned count; std::string first, last; };
static std::vector<Row> rows;

static void Initialize(ID3D12CommandQueue* queue) {
    if (FAILED(queue->GetTimestampFrequency(&frequency))) exit(30);
    ID3D12Device* device = nullptr;
    if (FAILED(queue->GetDevice(IID_PPV_ARGS(&device)))) exit(31);
    D3D12_QUERY_HEAP_DESC desc{};
    desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    desc.Count = capacity;
    if (FAILED(device->CreateQueryHeap(&desc, IID_PPV_ARGS(&heap)))) exit(32);
    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC buffer{};
    buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Width = capacity * sizeof(UINT64);
    buffer.Height = 1;
    buffer.DepthOrArraySize = buffer.MipLevels = 1;
    buffer.SampleDesc.Count = 1;
    buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    if (FAILED(device->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE,
            &buffer, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)))) exit(33);
    device->Release();
}

static unsigned Begin(ID3D12GraphicsCommandList* cmd, unsigned count,
                      const std::string& first, const std::string& last) {
    if (!heap || rows.size() * 2 + 2 > capacity) exit(34);
    unsigned index = static_cast<unsigned>(rows.size()) * 2;
    rows.push_back({count, first, last});
    cmd->EndQuery(heap, D3D12_QUERY_TYPE_TIMESTAMP, index);
    return index;
}

static void End(ID3D12GraphicsCommandList* cmd, unsigned index) {
    cmd->EndQuery(heap, D3D12_QUERY_TYPE_TIMESTAMP, index + 1);
    cmd->ResolveQueryData(heap, D3D12_QUERY_TYPE_TIMESTAMP, index, 2,
                          readback, index * sizeof(UINT64));
}

// Called only after the probe has waited for all submitted frames to complete.
static void Finish() {
    if (!readback || !completed) return;
    UINT64* ticks = nullptr;
    D3D12_RANGE range{0, rows.size() * 2 * sizeof(UINT64)};
    if (FAILED(readback->Map(0, &range, reinterpret_cast<void**>(&ticks)))) exit(35);
    std::ofstream out("chain-timings.tsv");
    out << "chain\tkernel_count\tstart_tick\tend_tick\tfrequency_hz\tgpu_us\tfirst\tlast\n";
    for (size_t i = 0; i < rows.size(); ++i) {
        out << i << '\t' << rows[i].count << '\t' << ticks[i * 2] << '\t'
            << ticks[i * 2 + 1] << '\t' << frequency << '\t'
            << double(ticks[i * 2 + 1] - ticks[i * 2]) * 1e6 / frequency << '\t'
            << rows[i].first << '\t' << rows[i].last << '\n';
    }
    D3D12_RANGE written{0, 0};
    readback->Unmap(0, &written);
    readback->Release();
    heap->Release();
}
}
