#pragma once

#include "Common/Precompile.h"

#include "Renderer/FrameData/FrameSequence.h"

namespace d14engine::renderer { struct DefaultBuffer; }

namespace d14engine::pipeline
{
    struct TextureSequence : renderer::FrameSequence<SharedPtr<renderer::DefaultBuffer>>
    {
        struct BufferInfo
        {
            UINT rootParamIndex;
            D3D12_GPU_VIRTUAL_ADDRESS gpuVirtAddr;
        };
        void draw(renderer::Renderer* rndr, const BufferInfo& info);
    };
}
