// Generated from installed Windows SDK signatures; isolated probe only.
using D3D_Close=HRESULT(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This);
static D3D_Close original_Close;
static HRESULT STDMETHODCALLTYPE Audit_Close(ID3D12GraphicsCommandList * This){Boundary(This,"Close");return original_Close(This);}
using D3D_Reset=HRESULT(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState);
static D3D_Reset original_Reset;
static HRESULT STDMETHODCALLTYPE Audit_Reset(ID3D12GraphicsCommandList * This, ID3D12CommandAllocator *pAllocator, ID3D12PipelineState *pInitialState){Boundary(This,"Reset");return original_Reset(This,pAllocator,pInitialState);}
using D3D_ClearState=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12PipelineState *pPipelineState);
static D3D_ClearState original_ClearState;
static void STDMETHODCALLTYPE Audit_ClearState(ID3D12GraphicsCommandList * This, ID3D12PipelineState *pPipelineState){Boundary(This,"ClearState");return original_ClearState(This,pPipelineState);}
using D3D_DrawInstanced=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
static D3D_DrawInstanced original_DrawInstanced;
static void STDMETHODCALLTYPE Audit_DrawInstanced(ID3D12GraphicsCommandList * This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation){Boundary(This,"DrawInstanced");return original_DrawInstanced(This,VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation);}
using D3D_DrawIndexedInstanced=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
static D3D_DrawIndexedInstanced original_DrawIndexedInstanced;
static void STDMETHODCALLTYPE Audit_DrawIndexedInstanced(ID3D12GraphicsCommandList * This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation){Boundary(This,"DrawIndexedInstanced");return original_DrawIndexedInstanced(This,IndexCountPerInstance,InstanceCount,StartIndexLocation,BaseVertexLocation,StartInstanceLocation);}
using D3D_Dispatch=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);
static D3D_Dispatch original_Dispatch;
static void STDMETHODCALLTYPE Audit_Dispatch(ID3D12GraphicsCommandList * This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ){Boundary(This,"Dispatch");return original_Dispatch(This,ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ);}
using D3D_CopyBufferRegion=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12Resource *pDstBuffer, UINT64 DstOffset, ID3D12Resource *pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes);
static D3D_CopyBufferRegion original_CopyBufferRegion;
static void STDMETHODCALLTYPE Audit_CopyBufferRegion(ID3D12GraphicsCommandList * This, ID3D12Resource *pDstBuffer, UINT64 DstOffset, ID3D12Resource *pSrcBuffer, UINT64 SrcOffset, UINT64 NumBytes){Boundary(This,"CopyBufferRegion");return original_CopyBufferRegion(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,NumBytes);}
using D3D_CopyTextureRegion=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc, const D3D12_BOX *pSrcBox);
static D3D_CopyTextureRegion original_CopyTextureRegion;
static void STDMETHODCALLTYPE Audit_CopyTextureRegion(ID3D12GraphicsCommandList * This, const D3D12_TEXTURE_COPY_LOCATION *pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc, const D3D12_BOX *pSrcBox){Boundary(This,"CopyTextureRegion");return original_CopyTextureRegion(This,pDst,DstX,DstY,DstZ,pSrc,pSrcBox);}
using D3D_CopyResource=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12Resource *pDstResource, ID3D12Resource *pSrcResource);
static D3D_CopyResource original_CopyResource;
static void STDMETHODCALLTYPE Audit_CopyResource(ID3D12GraphicsCommandList * This, ID3D12Resource *pDstResource, ID3D12Resource *pSrcResource){Boundary(This,"CopyResource");return original_CopyResource(This,pDstResource,pSrcResource);}
using D3D_CopyTiles=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12Resource *pTiledResource, const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE *pTileRegionSize, ID3D12Resource *pBuffer, UINT64 BufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS Flags);
static D3D_CopyTiles original_CopyTiles;
static void STDMETHODCALLTYPE Audit_CopyTiles(ID3D12GraphicsCommandList * This, ID3D12Resource *pTiledResource, const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE *pTileRegionSize, ID3D12Resource *pBuffer, UINT64 BufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS Flags){Boundary(This,"CopyTiles");return original_CopyTiles(This,pTiledResource,pTileRegionStartCoordinate,pTileRegionSize,pBuffer,BufferStartOffsetInBytes,Flags);}
using D3D_ResolveSubresource=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12Resource *pDstResource, UINT DstSubresource, ID3D12Resource *pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format);
static D3D_ResolveSubresource original_ResolveSubresource;
static void STDMETHODCALLTYPE Audit_ResolveSubresource(ID3D12GraphicsCommandList * This, ID3D12Resource *pDstResource, UINT DstSubresource, ID3D12Resource *pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format){Boundary(This,"ResolveSubresource");return original_ResolveSubresource(This,pDstResource,DstSubresource,pSrcResource,SrcSubresource,Format);}
using D3D_IASetPrimitiveTopology=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology);
static D3D_IASetPrimitiveTopology original_IASetPrimitiveTopology;
static void STDMETHODCALLTYPE Audit_IASetPrimitiveTopology(ID3D12GraphicsCommandList * This, D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology){Boundary(This,"IASetPrimitiveTopology");return original_IASetPrimitiveTopology(This,PrimitiveTopology);}
using D3D_RSSetViewports=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT NumViewports, const D3D12_VIEWPORT *pViewports);
static D3D_RSSetViewports original_RSSetViewports;
static void STDMETHODCALLTYPE Audit_RSSetViewports(ID3D12GraphicsCommandList * This, UINT NumViewports, const D3D12_VIEWPORT *pViewports){Boundary(This,"RSSetViewports");return original_RSSetViewports(This,NumViewports,pViewports);}
using D3D_RSSetScissorRects=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT NumRects, const D3D12_RECT *pRects);
static D3D_RSSetScissorRects original_RSSetScissorRects;
static void STDMETHODCALLTYPE Audit_RSSetScissorRects(ID3D12GraphicsCommandList * This, UINT NumRects, const D3D12_RECT *pRects){Boundary(This,"RSSetScissorRects");return original_RSSetScissorRects(This,NumRects,pRects);}
using D3D_OMSetBlendFactor=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, const FLOAT BlendFactor[ 4 ]);
static D3D_OMSetBlendFactor original_OMSetBlendFactor;
static void STDMETHODCALLTYPE Audit_OMSetBlendFactor(ID3D12GraphicsCommandList * This, const FLOAT BlendFactor[ 4 ]){Boundary(This,"OMSetBlendFactor");return original_OMSetBlendFactor(This,BlendFactor);}
using D3D_OMSetStencilRef=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT StencilRef);
static D3D_OMSetStencilRef original_OMSetStencilRef;
static void STDMETHODCALLTYPE Audit_OMSetStencilRef(ID3D12GraphicsCommandList * This, UINT StencilRef){Boundary(This,"OMSetStencilRef");return original_OMSetStencilRef(This,StencilRef);}
using D3D_SetPipelineState=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12PipelineState *pPipelineState);
static D3D_SetPipelineState original_SetPipelineState;
static void STDMETHODCALLTYPE Audit_SetPipelineState(ID3D12GraphicsCommandList * This, ID3D12PipelineState *pPipelineState){Boundary(This,"SetPipelineState");return original_SetPipelineState(This,pPipelineState);}
using D3D_ResourceBarrier=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT NumBarriers, const D3D12_RESOURCE_BARRIER *pBarriers);
static D3D_ResourceBarrier original_ResourceBarrier;
static void STDMETHODCALLTYPE Audit_ResourceBarrier(ID3D12GraphicsCommandList * This, UINT NumBarriers, const D3D12_RESOURCE_BARRIER *pBarriers){Boundary(This,"ResourceBarrier");return original_ResourceBarrier(This,NumBarriers,pBarriers);}
using D3D_ExecuteBundle=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12GraphicsCommandList *pCommandList);
static D3D_ExecuteBundle original_ExecuteBundle;
static void STDMETHODCALLTYPE Audit_ExecuteBundle(ID3D12GraphicsCommandList * This, ID3D12GraphicsCommandList *pCommandList){Boundary(This,"ExecuteBundle");return original_ExecuteBundle(This,pCommandList);}
using D3D_SetDescriptorHeaps=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT NumDescriptorHeaps, ID3D12DescriptorHeap *const *ppDescriptorHeaps);
static D3D_SetDescriptorHeaps original_SetDescriptorHeaps;
static void STDMETHODCALLTYPE Audit_SetDescriptorHeaps(ID3D12GraphicsCommandList * This, UINT NumDescriptorHeaps, ID3D12DescriptorHeap *const *ppDescriptorHeaps){Boundary(This,"SetDescriptorHeaps");return original_SetDescriptorHeaps(This,NumDescriptorHeaps,ppDescriptorHeaps);}
using D3D_SetComputeRootSignature=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12RootSignature *pRootSignature);
static D3D_SetComputeRootSignature original_SetComputeRootSignature;
static void STDMETHODCALLTYPE Audit_SetComputeRootSignature(ID3D12GraphicsCommandList * This, ID3D12RootSignature *pRootSignature){Boundary(This,"SetComputeRootSignature");return original_SetComputeRootSignature(This,pRootSignature);}
using D3D_SetGraphicsRootSignature=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12RootSignature *pRootSignature);
static D3D_SetGraphicsRootSignature original_SetGraphicsRootSignature;
static void STDMETHODCALLTYPE Audit_SetGraphicsRootSignature(ID3D12GraphicsCommandList * This, ID3D12RootSignature *pRootSignature){Boundary(This,"SetGraphicsRootSignature");return original_SetGraphicsRootSignature(This,pRootSignature);}
using D3D_SetComputeRootDescriptorTable=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
static D3D_SetComputeRootDescriptorTable original_SetComputeRootDescriptorTable;
static void STDMETHODCALLTYPE Audit_SetComputeRootDescriptorTable(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor){Boundary(This,"SetComputeRootDescriptorTable");return original_SetComputeRootDescriptorTable(This,RootParameterIndex,BaseDescriptor);}
using D3D_SetGraphicsRootDescriptorTable=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
static D3D_SetGraphicsRootDescriptorTable original_SetGraphicsRootDescriptorTable;
static void STDMETHODCALLTYPE Audit_SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor){Boundary(This,"SetGraphicsRootDescriptorTable");return original_SetGraphicsRootDescriptorTable(This,RootParameterIndex,BaseDescriptor);}
using D3D_SetComputeRoot32BitConstant=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues);
static D3D_SetComputeRoot32BitConstant original_SetComputeRoot32BitConstant;
static void STDMETHODCALLTYPE Audit_SetComputeRoot32BitConstant(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues){Boundary(This,"SetComputeRoot32BitConstant");return original_SetComputeRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues);}
using D3D_SetGraphicsRoot32BitConstant=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues);
static D3D_SetGraphicsRoot32BitConstant original_SetGraphicsRoot32BitConstant;
static void STDMETHODCALLTYPE Audit_SetGraphicsRoot32BitConstant(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues){Boundary(This,"SetGraphicsRoot32BitConstant");return original_SetGraphicsRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues);}
using D3D_SetComputeRoot32BitConstants=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT Num32BitValuesToSet, const void *pSrcData, UINT DestOffsetIn32BitValues);
static D3D_SetComputeRoot32BitConstants original_SetComputeRoot32BitConstants;
static void STDMETHODCALLTYPE Audit_SetComputeRoot32BitConstants(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT Num32BitValuesToSet, const void *pSrcData, UINT DestOffsetIn32BitValues){Boundary(This,"SetComputeRoot32BitConstants");return original_SetComputeRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues);}
using D3D_SetGraphicsRoot32BitConstants=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT Num32BitValuesToSet, const void *pSrcData, UINT DestOffsetIn32BitValues);
static D3D_SetGraphicsRoot32BitConstants original_SetGraphicsRoot32BitConstants;
static void STDMETHODCALLTYPE Audit_SetGraphicsRoot32BitConstants(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, UINT Num32BitValuesToSet, const void *pSrcData, UINT DestOffsetIn32BitValues){Boundary(This,"SetGraphicsRoot32BitConstants");return original_SetGraphicsRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues);}
using D3D_SetComputeRootConstantBufferView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
static D3D_SetComputeRootConstantBufferView original_SetComputeRootConstantBufferView;
static void STDMETHODCALLTYPE Audit_SetComputeRootConstantBufferView(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation){Boundary(This,"SetComputeRootConstantBufferView");return original_SetComputeRootConstantBufferView(This,RootParameterIndex,BufferLocation);}
using D3D_SetGraphicsRootConstantBufferView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
static D3D_SetGraphicsRootConstantBufferView original_SetGraphicsRootConstantBufferView;
static void STDMETHODCALLTYPE Audit_SetGraphicsRootConstantBufferView(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation){Boundary(This,"SetGraphicsRootConstantBufferView");return original_SetGraphicsRootConstantBufferView(This,RootParameterIndex,BufferLocation);}
using D3D_SetComputeRootShaderResourceView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
static D3D_SetComputeRootShaderResourceView original_SetComputeRootShaderResourceView;
static void STDMETHODCALLTYPE Audit_SetComputeRootShaderResourceView(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation){Boundary(This,"SetComputeRootShaderResourceView");return original_SetComputeRootShaderResourceView(This,RootParameterIndex,BufferLocation);}
using D3D_SetGraphicsRootShaderResourceView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
static D3D_SetGraphicsRootShaderResourceView original_SetGraphicsRootShaderResourceView;
static void STDMETHODCALLTYPE Audit_SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation){Boundary(This,"SetGraphicsRootShaderResourceView");return original_SetGraphicsRootShaderResourceView(This,RootParameterIndex,BufferLocation);}
using D3D_SetComputeRootUnorderedAccessView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
static D3D_SetComputeRootUnorderedAccessView original_SetComputeRootUnorderedAccessView;
static void STDMETHODCALLTYPE Audit_SetComputeRootUnorderedAccessView(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation){Boundary(This,"SetComputeRootUnorderedAccessView");return original_SetComputeRootUnorderedAccessView(This,RootParameterIndex,BufferLocation);}
using D3D_SetGraphicsRootUnorderedAccessView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
static D3D_SetGraphicsRootUnorderedAccessView original_SetGraphicsRootUnorderedAccessView;
static void STDMETHODCALLTYPE Audit_SetGraphicsRootUnorderedAccessView(ID3D12GraphicsCommandList * This, UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation){Boundary(This,"SetGraphicsRootUnorderedAccessView");return original_SetGraphicsRootUnorderedAccessView(This,RootParameterIndex,BufferLocation);}
using D3D_IASetIndexBuffer=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, const D3D12_INDEX_BUFFER_VIEW *pView);
static D3D_IASetIndexBuffer original_IASetIndexBuffer;
static void STDMETHODCALLTYPE Audit_IASetIndexBuffer(ID3D12GraphicsCommandList * This, const D3D12_INDEX_BUFFER_VIEW *pView){Boundary(This,"IASetIndexBuffer");return original_IASetIndexBuffer(This,pView);}
using D3D_IASetVertexBuffers=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW *pViews);
static D3D_IASetVertexBuffers original_IASetVertexBuffers;
static void STDMETHODCALLTYPE Audit_IASetVertexBuffers(ID3D12GraphicsCommandList * This, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW *pViews){Boundary(This,"IASetVertexBuffers");return original_IASetVertexBuffers(This,StartSlot,NumViews,pViews);}
using D3D_SOSetTargets=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT StartSlot, UINT NumViews, const D3D12_STREAM_OUTPUT_BUFFER_VIEW *pViews);
static D3D_SOSetTargets original_SOSetTargets;
static void STDMETHODCALLTYPE Audit_SOSetTargets(ID3D12GraphicsCommandList * This, UINT StartSlot, UINT NumViews, const D3D12_STREAM_OUTPUT_BUFFER_VIEW *pViews){Boundary(This,"SOSetTargets");return original_SOSetTargets(This,StartSlot,NumViews,pViews);}
using D3D_OMSetRenderTargets=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor);
static D3D_OMSetRenderTargets original_OMSetRenderTargets;
static void STDMETHODCALLTYPE Audit_OMSetRenderTargets(ID3D12GraphicsCommandList * This, UINT NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor){Boundary(This,"OMSetRenderTargets");return original_OMSetRenderTargets(This,NumRenderTargetDescriptors,pRenderTargetDescriptors,RTsSingleHandleToDescriptorRange,pDepthStencilDescriptor);}
using D3D_ClearDepthStencilView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT *pRects);
static D3D_ClearDepthStencilView original_ClearDepthStencilView;
static void STDMETHODCALLTYPE Audit_ClearDepthStencilView(ID3D12GraphicsCommandList * This, D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT *pRects){Boundary(This,"ClearDepthStencilView");return original_ClearDepthStencilView(This,DepthStencilView,ClearFlags,Depth,Stencil,NumRects,pRects);}
using D3D_ClearRenderTargetView=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[ 4 ], UINT NumRects, const D3D12_RECT *pRects);
static D3D_ClearRenderTargetView original_ClearRenderTargetView;
static void STDMETHODCALLTYPE Audit_ClearRenderTargetView(ID3D12GraphicsCommandList * This, D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[ 4 ], UINT NumRects, const D3D12_RECT *pRects){Boundary(This,"ClearRenderTargetView");return original_ClearRenderTargetView(This,RenderTargetView,ColorRGBA,NumRects,pRects);}
using D3D_ClearUnorderedAccessViewUint=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource *pResource, const UINT Values[ 4 ], UINT NumRects, const D3D12_RECT *pRects);
static D3D_ClearUnorderedAccessViewUint original_ClearUnorderedAccessViewUint;
static void STDMETHODCALLTYPE Audit_ClearUnorderedAccessViewUint(ID3D12GraphicsCommandList * This, D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource *pResource, const UINT Values[ 4 ], UINT NumRects, const D3D12_RECT *pRects){Boundary(This,"ClearUnorderedAccessViewUint");return original_ClearUnorderedAccessViewUint(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects);}
using D3D_ClearUnorderedAccessViewFloat=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource *pResource, const FLOAT Values[ 4 ], UINT NumRects, const D3D12_RECT *pRects);
static D3D_ClearUnorderedAccessViewFloat original_ClearUnorderedAccessViewFloat;
static void STDMETHODCALLTYPE Audit_ClearUnorderedAccessViewFloat(ID3D12GraphicsCommandList * This, D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle, ID3D12Resource *pResource, const FLOAT Values[ 4 ], UINT NumRects, const D3D12_RECT *pRects){Boundary(This,"ClearUnorderedAccessViewFloat");return original_ClearUnorderedAccessViewFloat(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects);}
using D3D_DiscardResource=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12Resource *pResource, const D3D12_DISCARD_REGION *pRegion);
static D3D_DiscardResource original_DiscardResource;
static void STDMETHODCALLTYPE Audit_DiscardResource(ID3D12GraphicsCommandList * This, ID3D12Resource *pResource, const D3D12_DISCARD_REGION *pRegion){Boundary(This,"DiscardResource");return original_DiscardResource(This,pResource,pRegion);}
using D3D_BeginQuery=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index);
static D3D_BeginQuery original_BeginQuery;
static void STDMETHODCALLTYPE Audit_BeginQuery(ID3D12GraphicsCommandList * This, ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index){Boundary(This,"BeginQuery");return original_BeginQuery(This,pQueryHeap,Type,Index);}
using D3D_EndQuery=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index);
static D3D_EndQuery original_EndQuery;
static void STDMETHODCALLTYPE Audit_EndQuery(ID3D12GraphicsCommandList * This, ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT Index){Boundary(This,"EndQuery");return original_EndQuery(This,pQueryHeap,Type,Index);}
using D3D_ResolveQueryData=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource *pDestinationBuffer, UINT64 AlignedDestinationBufferOffset);
static D3D_ResolveQueryData original_ResolveQueryData;
static void STDMETHODCALLTYPE Audit_ResolveQueryData(ID3D12GraphicsCommandList * This, ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource *pDestinationBuffer, UINT64 AlignedDestinationBufferOffset){Boundary(This,"ResolveQueryData");return original_ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset);}
using D3D_SetPredication=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12Resource *pBuffer, UINT64 AlignedBufferOffset, D3D12_PREDICATION_OP Operation);
static D3D_SetPredication original_SetPredication;
static void STDMETHODCALLTYPE Audit_SetPredication(ID3D12GraphicsCommandList * This, ID3D12Resource *pBuffer, UINT64 AlignedBufferOffset, D3D12_PREDICATION_OP Operation){Boundary(This,"SetPredication");return original_SetPredication(This,pBuffer,AlignedBufferOffset,Operation);}
using D3D_SetMarker=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT Metadata, const void *pData, UINT Size);
static D3D_SetMarker original_SetMarker;
static void STDMETHODCALLTYPE Audit_SetMarker(ID3D12GraphicsCommandList * This, UINT Metadata, const void *pData, UINT Size){Boundary(This,"SetMarker");return original_SetMarker(This,Metadata,pData,Size);}
using D3D_BeginEvent=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, UINT Metadata, const void *pData, UINT Size);
static D3D_BeginEvent original_BeginEvent;
static void STDMETHODCALLTYPE Audit_BeginEvent(ID3D12GraphicsCommandList * This, UINT Metadata, const void *pData, UINT Size){Boundary(This,"BeginEvent");return original_BeginEvent(This,Metadata,pData,Size);}
using D3D_EndEvent=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This);
static D3D_EndEvent original_EndEvent;
static void STDMETHODCALLTYPE Audit_EndEvent(ID3D12GraphicsCommandList * This){Boundary(This,"EndEvent");return original_EndEvent(This);}
using D3D_ExecuteIndirect=void(STDMETHODCALLTYPE*)(ID3D12GraphicsCommandList * This, ID3D12CommandSignature *pCommandSignature, UINT MaxCommandCount, ID3D12Resource *pArgumentBuffer, UINT64 ArgumentBufferOffset, ID3D12Resource *pCountBuffer, UINT64 CountBufferOffset);
static D3D_ExecuteIndirect original_ExecuteIndirect;
static void STDMETHODCALLTYPE Audit_ExecuteIndirect(ID3D12GraphicsCommandList * This, ID3D12CommandSignature *pCommandSignature, UINT MaxCommandCount, ID3D12Resource *pArgumentBuffer, UINT64 ArgumentBufferOffset, ID3D12Resource *pCountBuffer, UINT64 CountBufferOffset){Boundary(This,"ExecuteIndirect");return original_ExecuteIndirect(This,pCommandSignature,MaxCommandCount,pArgumentBuffer,ArgumentBufferOffset,pCountBuffer,CountBufferOffset);}
static void AttachD3D(ID3D12GraphicsCommandList*c){auto v=*reinterpret_cast<void***>(c);
original_Close=reinterpret_cast<D3D_Close>(v[9]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_Close),Audit_Close)!=NO_ERROR)exit(60);
original_Reset=reinterpret_cast<D3D_Reset>(v[10]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_Reset),Audit_Reset)!=NO_ERROR)exit(60);
original_ClearState=reinterpret_cast<D3D_ClearState>(v[11]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ClearState),Audit_ClearState)!=NO_ERROR)exit(60);
original_DrawInstanced=reinterpret_cast<D3D_DrawInstanced>(v[12]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_DrawInstanced),Audit_DrawInstanced)!=NO_ERROR)exit(60);
original_DrawIndexedInstanced=reinterpret_cast<D3D_DrawIndexedInstanced>(v[13]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_DrawIndexedInstanced),Audit_DrawIndexedInstanced)!=NO_ERROR)exit(60);
original_Dispatch=reinterpret_cast<D3D_Dispatch>(v[14]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_Dispatch),Audit_Dispatch)!=NO_ERROR)exit(60);
original_CopyBufferRegion=reinterpret_cast<D3D_CopyBufferRegion>(v[15]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_CopyBufferRegion),Audit_CopyBufferRegion)!=NO_ERROR)exit(60);
original_CopyTextureRegion=reinterpret_cast<D3D_CopyTextureRegion>(v[16]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_CopyTextureRegion),Audit_CopyTextureRegion)!=NO_ERROR)exit(60);
original_CopyResource=reinterpret_cast<D3D_CopyResource>(v[17]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_CopyResource),Audit_CopyResource)!=NO_ERROR)exit(60);
original_CopyTiles=reinterpret_cast<D3D_CopyTiles>(v[18]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_CopyTiles),Audit_CopyTiles)!=NO_ERROR)exit(60);
original_ResolveSubresource=reinterpret_cast<D3D_ResolveSubresource>(v[19]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ResolveSubresource),Audit_ResolveSubresource)!=NO_ERROR)exit(60);
original_IASetPrimitiveTopology=reinterpret_cast<D3D_IASetPrimitiveTopology>(v[20]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_IASetPrimitiveTopology),Audit_IASetPrimitiveTopology)!=NO_ERROR)exit(60);
original_RSSetViewports=reinterpret_cast<D3D_RSSetViewports>(v[21]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_RSSetViewports),Audit_RSSetViewports)!=NO_ERROR)exit(60);
original_RSSetScissorRects=reinterpret_cast<D3D_RSSetScissorRects>(v[22]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_RSSetScissorRects),Audit_RSSetScissorRects)!=NO_ERROR)exit(60);
original_OMSetBlendFactor=reinterpret_cast<D3D_OMSetBlendFactor>(v[23]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_OMSetBlendFactor),Audit_OMSetBlendFactor)!=NO_ERROR)exit(60);
original_OMSetStencilRef=reinterpret_cast<D3D_OMSetStencilRef>(v[24]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_OMSetStencilRef),Audit_OMSetStencilRef)!=NO_ERROR)exit(60);
original_SetPipelineState=reinterpret_cast<D3D_SetPipelineState>(v[25]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetPipelineState),Audit_SetPipelineState)!=NO_ERROR)exit(60);
original_ResourceBarrier=reinterpret_cast<D3D_ResourceBarrier>(v[26]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ResourceBarrier),Audit_ResourceBarrier)!=NO_ERROR)exit(60);
original_ExecuteBundle=reinterpret_cast<D3D_ExecuteBundle>(v[27]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ExecuteBundle),Audit_ExecuteBundle)!=NO_ERROR)exit(60);
original_SetDescriptorHeaps=reinterpret_cast<D3D_SetDescriptorHeaps>(v[28]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetDescriptorHeaps),Audit_SetDescriptorHeaps)!=NO_ERROR)exit(60);
original_SetComputeRootSignature=reinterpret_cast<D3D_SetComputeRootSignature>(v[29]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRootSignature),Audit_SetComputeRootSignature)!=NO_ERROR)exit(60);
original_SetGraphicsRootSignature=reinterpret_cast<D3D_SetGraphicsRootSignature>(v[30]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRootSignature),Audit_SetGraphicsRootSignature)!=NO_ERROR)exit(60);
original_SetComputeRootDescriptorTable=reinterpret_cast<D3D_SetComputeRootDescriptorTable>(v[31]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRootDescriptorTable),Audit_SetComputeRootDescriptorTable)!=NO_ERROR)exit(60);
original_SetGraphicsRootDescriptorTable=reinterpret_cast<D3D_SetGraphicsRootDescriptorTable>(v[32]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRootDescriptorTable),Audit_SetGraphicsRootDescriptorTable)!=NO_ERROR)exit(60);
original_SetComputeRoot32BitConstant=reinterpret_cast<D3D_SetComputeRoot32BitConstant>(v[33]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRoot32BitConstant),Audit_SetComputeRoot32BitConstant)!=NO_ERROR)exit(60);
original_SetGraphicsRoot32BitConstant=reinterpret_cast<D3D_SetGraphicsRoot32BitConstant>(v[34]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRoot32BitConstant),Audit_SetGraphicsRoot32BitConstant)!=NO_ERROR)exit(60);
original_SetComputeRoot32BitConstants=reinterpret_cast<D3D_SetComputeRoot32BitConstants>(v[35]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRoot32BitConstants),Audit_SetComputeRoot32BitConstants)!=NO_ERROR)exit(60);
original_SetGraphicsRoot32BitConstants=reinterpret_cast<D3D_SetGraphicsRoot32BitConstants>(v[36]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRoot32BitConstants),Audit_SetGraphicsRoot32BitConstants)!=NO_ERROR)exit(60);
original_SetComputeRootConstantBufferView=reinterpret_cast<D3D_SetComputeRootConstantBufferView>(v[37]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRootConstantBufferView),Audit_SetComputeRootConstantBufferView)!=NO_ERROR)exit(60);
original_SetGraphicsRootConstantBufferView=reinterpret_cast<D3D_SetGraphicsRootConstantBufferView>(v[38]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRootConstantBufferView),Audit_SetGraphicsRootConstantBufferView)!=NO_ERROR)exit(60);
original_SetComputeRootShaderResourceView=reinterpret_cast<D3D_SetComputeRootShaderResourceView>(v[39]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRootShaderResourceView),Audit_SetComputeRootShaderResourceView)!=NO_ERROR)exit(60);
original_SetGraphicsRootShaderResourceView=reinterpret_cast<D3D_SetGraphicsRootShaderResourceView>(v[40]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRootShaderResourceView),Audit_SetGraphicsRootShaderResourceView)!=NO_ERROR)exit(60);
original_SetComputeRootUnorderedAccessView=reinterpret_cast<D3D_SetComputeRootUnorderedAccessView>(v[41]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetComputeRootUnorderedAccessView),Audit_SetComputeRootUnorderedAccessView)!=NO_ERROR)exit(60);
original_SetGraphicsRootUnorderedAccessView=reinterpret_cast<D3D_SetGraphicsRootUnorderedAccessView>(v[42]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetGraphicsRootUnorderedAccessView),Audit_SetGraphicsRootUnorderedAccessView)!=NO_ERROR)exit(60);
original_IASetIndexBuffer=reinterpret_cast<D3D_IASetIndexBuffer>(v[43]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_IASetIndexBuffer),Audit_IASetIndexBuffer)!=NO_ERROR)exit(60);
original_IASetVertexBuffers=reinterpret_cast<D3D_IASetVertexBuffers>(v[44]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_IASetVertexBuffers),Audit_IASetVertexBuffers)!=NO_ERROR)exit(60);
original_SOSetTargets=reinterpret_cast<D3D_SOSetTargets>(v[45]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SOSetTargets),Audit_SOSetTargets)!=NO_ERROR)exit(60);
original_OMSetRenderTargets=reinterpret_cast<D3D_OMSetRenderTargets>(v[46]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_OMSetRenderTargets),Audit_OMSetRenderTargets)!=NO_ERROR)exit(60);
original_ClearDepthStencilView=reinterpret_cast<D3D_ClearDepthStencilView>(v[47]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ClearDepthStencilView),Audit_ClearDepthStencilView)!=NO_ERROR)exit(60);
original_ClearRenderTargetView=reinterpret_cast<D3D_ClearRenderTargetView>(v[48]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ClearRenderTargetView),Audit_ClearRenderTargetView)!=NO_ERROR)exit(60);
original_ClearUnorderedAccessViewUint=reinterpret_cast<D3D_ClearUnorderedAccessViewUint>(v[49]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ClearUnorderedAccessViewUint),Audit_ClearUnorderedAccessViewUint)!=NO_ERROR)exit(60);
original_ClearUnorderedAccessViewFloat=reinterpret_cast<D3D_ClearUnorderedAccessViewFloat>(v[50]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ClearUnorderedAccessViewFloat),Audit_ClearUnorderedAccessViewFloat)!=NO_ERROR)exit(60);
original_DiscardResource=reinterpret_cast<D3D_DiscardResource>(v[51]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_DiscardResource),Audit_DiscardResource)!=NO_ERROR)exit(60);
original_BeginQuery=reinterpret_cast<D3D_BeginQuery>(v[52]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_BeginQuery),Audit_BeginQuery)!=NO_ERROR)exit(60);
original_EndQuery=reinterpret_cast<D3D_EndQuery>(v[53]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_EndQuery),Audit_EndQuery)!=NO_ERROR)exit(60);
original_ResolveQueryData=reinterpret_cast<D3D_ResolveQueryData>(v[54]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ResolveQueryData),Audit_ResolveQueryData)!=NO_ERROR)exit(60);
original_SetPredication=reinterpret_cast<D3D_SetPredication>(v[55]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetPredication),Audit_SetPredication)!=NO_ERROR)exit(60);
original_SetMarker=reinterpret_cast<D3D_SetMarker>(v[56]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_SetMarker),Audit_SetMarker)!=NO_ERROR)exit(60);
original_BeginEvent=reinterpret_cast<D3D_BeginEvent>(v[57]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_BeginEvent),Audit_BeginEvent)!=NO_ERROR)exit(60);
original_EndEvent=reinterpret_cast<D3D_EndEvent>(v[58]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_EndEvent),Audit_EndEvent)!=NO_ERROR)exit(60);
original_ExecuteIndirect=reinterpret_cast<D3D_ExecuteIndirect>(v[59]);
if(DetourAttach(reinterpret_cast<PVOID*>(&original_ExecuteIndirect),Audit_ExecuteIndirect)!=NO_ERROR)exit(60);
}
