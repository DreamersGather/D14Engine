#include "Common/Precompile.h"

#include "Renderer/GraphUtils/Resource.h"

#include "Common/DirectXError.h"

#include "Renderer/GraphUtils/Barrier.h"

namespace d14engine::renderer::graph_utils
{
    ComPtr<ID3D12Resource> capture(ID3D12GraphicsCommandList* cmdList,
        ID3D12Resource* texture, D3D12_RESOURCE_STATES initState)
    {
        ComPtr<ID3D12Device> device = {};
        THROW_IF_FAILED(texture->GetDevice(IID_PPV_ARGS(&device)));

        ///////////////////////////
        // Get Texture Footprint //
        ///////////////////////////

        auto textureDesc = texture->GetDesc();
        UINT64 totalBytes = 0, rowSizeInBytes = 0;

        device->GetCopyableFootprints
        (
        /* pResourceDesc    */ &textureDesc,
        /* FirstSubresource */ 0,
        /* NumSubresources  */ 1,
        /* BaseOffset       */ 0,
        /* pLayouts         */ nullptr,
        /* pNumRows         */ nullptr,
        /* pRowSizeInBytes  */ &rowSizeInBytes,
        /* pTotalBytes      */ &totalBytes
        );

        ///////////////////////////
        // Create Staging Buffer //
        ///////////////////////////

        ComPtr<ID3D12Resource> staging = {};
        auto stagingProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        auto stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);

        THROW_IF_FAILED(device->CreateCommittedResource
        (
        /* pHeapProperties      */ &stagingProp,
        /* HeapFlags            */ D3D12_HEAP_FLAG_NONE,
        /* pDesc                */ &stagingDesc,
        /* InitialResourceState */ D3D12_RESOURCE_STATE_COPY_DEST,
        /* pOptimizedClearValue */ nullptr,
        /* riidResource         */
        /* ppvResource          */ IID_PPV_ARGS(&staging)
        ));

        //////////////////////////
        // Setup Copy Footprint //
        //////////////////////////

        CD3DX12_TEXTURE_COPY_LOCATION src(texture, 0);

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        footprint.Footprint =
        {
            .Format   = textureDesc.Format,
            .Width    = (UINT)textureDesc.Width,
            .Height   = (UINT)textureDesc.Height,
            .Depth    = (UINT)textureDesc.DepthOrArraySize,
            .RowPitch = (UINT)rowSizeInBytes
        };
        CD3DX12_TEXTURE_COPY_LOCATION dst(staging.Get(), footprint);

        ////////////////////////////
        // Copy Texture to Buffer //
        ////////////////////////////

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition
        (
            texture, initState, D3D12_RESOURCE_STATE_COPY_SOURCE
        );
        cmdList->ResourceBarrier(1, &barrier);

        cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        graph_utils::revertBarrier(1, &barrier);
        cmdList->ResourceBarrier(1, &barrier);

        return staging;
    }
}
