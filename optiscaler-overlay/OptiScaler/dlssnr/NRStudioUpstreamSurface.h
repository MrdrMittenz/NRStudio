#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <cstring>
#include <vector>

namespace NRStudioUpstream {
// Descriptors only name private resources and are immutable across frames.
// Retired allocations stay alive until explicit GPU-idle cleanup.
class Surface {
    struct Allocation {
        ID3D12Resource *colour = nullptr, *packed = nullptr;
        ID3D12DescriptorHeap* heap = nullptr;
        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST;
        bool packedReadable = false;
        void release() {
            if (colour) colour->Release();
            if (packed) packed->Release();
            if (heap) heap->Release();
            *this = {};
        }
    } current_;
    std::vector<Allocation> retained_;
    ID3D12RootSignature* root_ = nullptr;
    ID3D12PipelineState* pipeline_ = nullptr;
    static void transition(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource,
                           D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
        if (from == to) return;
        D3D12_RESOURCE_BARRIER b{};
        b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        b.Transition = {resource, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, from, to};
        cmd->ResourceBarrier(1, &b);
    }
    HRESULT pipeline(ID3D12Device* device) {
        if (pipeline_) return S_OK;
        D3D12_DESCRIPTOR_RANGE ranges[2]{};
        ranges[0] = {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0};
        ranges[1] = {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 1};
        D3D12_ROOT_PARAMETER param{};
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.DescriptorTable = {2, ranges};
        D3D12_ROOT_SIGNATURE_DESC rd{}; rd.NumParameters = 1; rd.pParameters = &param;
        ID3DBlob *blob = nullptr, *errors = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rd, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errors);
        if (errors) errors->Release();
        if (FAILED(hr)) return hr;
        if (!root_) hr = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_));
        blob->Release(); if (FAILED(hr)) return hr;
        // No filtering, exposure, clamping or tone curve. All finite R11 values
        // are exactly representable in FP16, including subnormal values.
        const char* shader = R"(
            Texture2D<float3> source : register(t0);
            RWTexture2D<float4> destination : register(u0);
            [numthreads(8,8,1)] void main(uint3 p : SV_DispatchThreadID) {
                uint w,h; destination.GetDimensions(w,h);
                if (p.x < w && p.y < h)
                    destination[p.xy] = float4(source.Load(int3(p.xy,0)),1.0);
            })";
        errors = nullptr;
        hr = D3DCompile(shader, std::strlen(shader), nullptr, nullptr, nullptr, "main", "cs_5_0",
                        D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &blob, &errors);
        if (errors) errors->Release();
        if (FAILED(hr)) return hr;
        D3D12_COMPUTE_PIPELINE_STATE_DESC pd{}; pd.pRootSignature = root_;
        pd.CS = {blob->GetBufferPointer(), blob->GetBufferSize()};
        hr = device->CreateComputePipelineState(&pd, IID_PPV_ARGS(&pipeline_));
        blob->Release(); return hr;
    }
public:
    Surface() = default;
    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    ID3D12Resource* get() const { return current_.colour; }
    static bool supported(const D3D12_RESOURCE_DESC& d, unsigned w, unsigned h,
                          unsigned x = 0, unsigned y = 0) {
        return w && h && d.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D
            && x <= d.Width && w <= d.Width - x && y <= d.Height && h <= d.Height - y
            && d.DepthOrArraySize == 1 && d.MipLevels == 1 && d.SampleDesc.Count == 1
            && (d.Format == DXGI_FORMAT_R16G16B16A16_FLOAT || d.Format == DXGI_FORMAT_R11G11B10_FLOAT);
    }
    HRESULT prepare(ID3D12GraphicsCommandList* cmd, ID3D12Resource* source,
                    unsigned w, unsigned h, unsigned x = 0, unsigned y = 0) {
        if (!cmd || !source || cmd->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT) return E_INVALIDARG;
        auto desc = source->GetDesc();
        if (!supported(desc, w, h, x, y)) return E_INVALIDARG;
        bool packed = desc.Format == DXGI_FORMAT_R11G11B10_FLOAT;
        if (!current_.colour || current_.colour->GetDesc().Width != w
            || current_.colour->GetDesc().Height != h || bool(current_.packed) != packed) {
            if (retained_.size() >= 8) return E_OUTOFMEMORY;
            ID3D12Device* device = nullptr;
            HRESULT hr = source->GetDevice(IID_PPV_ARGS(&device));
            if (FAILED(hr)) return hr;
            Allocation next;
            desc.Width = w; desc.Height = h; desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.Alignment = 0; desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            D3D12_HEAP_PROPERTIES hp{}; hp.Type = D3D12_HEAP_TYPE_DEFAULT;
            hr = device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&next.colour));
            if (SUCCEEDED(hr) && packed) {
                desc.Flags = D3D12_RESOURCE_FLAG_NONE; desc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
                hr = device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &desc,
                    D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&next.packed));
                if (SUCCEEDED(hr)) hr = pipeline(device);
                D3D12_DESCRIPTOR_HEAP_DESC hd{}; hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                hd.NumDescriptors = 2; hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (SUCCEEDED(hr)) hr = device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&next.heap));
                if (SUCCEEDED(hr)) {
                    auto handle = next.heap->GetCPUDescriptorHandleForHeapStart();
                    device->CreateShaderResourceView(next.packed, nullptr, handle);
                    handle.ptr += device->GetDescriptorHandleIncrementSize(hd.Type);
                    device->CreateUnorderedAccessView(next.colour, nullptr, nullptr, handle);
                }
            }
            device->Release();
            if (FAILED(hr)) { next.release(); return hr; }
            if (current_.colour) retained_.push_back(current_);
            current_ = next;
        }
        auto* destination = packed ? current_.packed : current_.colour;
        auto from = packed ? (current_.packedReadable ? D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
                           : D3D12_RESOURCE_STATE_COPY_DEST) : current_.state;
        transition(cmd, source, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
        transition(cmd, destination, from, D3D12_RESOURCE_STATE_COPY_DEST);
        D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
        src.pResource = source; src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.pResource = destination; dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        D3D12_BOX box{x, y, 0, x+w, y+h, 1};
        cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);
        transition(cmd, source, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        if (packed) {
            transition(cmd, destination, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            current_.packedReadable = true;
            transition(cmd, current_.colour, current_.state, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            cmd->SetDescriptorHeaps(1, &current_.heap);
            cmd->SetComputeRootSignature(root_); cmd->SetPipelineState(pipeline_);
            cmd->SetComputeRootDescriptorTable(0, current_.heap->GetGPUDescriptorHandleForHeapStart());
            cmd->Dispatch((w+7)/8, (h+7)/8, 1);
            D3D12_RESOURCE_BARRIER b{}; b.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            b.UAV.pResource = current_.colour; cmd->ResourceBarrier(1, &b);
        } else transition(cmd, current_.colour, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        current_.state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        return S_OK;
    }
    void readable(ID3D12GraphicsCommandList* cmd) {
        transition(cmd, current_.colour, current_.state, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        current_.state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
    void release_after_gpu_idle() {
        current_.release();
        for (auto& allocation : retained_) allocation.release();
        retained_.clear();
        if (pipeline_) pipeline_->Release(); pipeline_ = nullptr;
        if (root_) root_->Release(); root_ = nullptr;
    }
};
}
