#pragma once

#include "Common/Precompile.h"

namespace d14engine::renderer::graph_utils
{
    // GPU Commands Required
    ComPtr<ID3D12Resource> capture(ID3D12GraphicsCommandList* cmdList,
        ID3D12Resource* texture, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
}
