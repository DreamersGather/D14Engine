#pragma once

#include "Common/Precompile.h"

#include "Pipeline/2D/TextureSequence.h"

#include "Renderer/Interfaces/DrawObject.h"

namespace d14engine::renderer { struct ConstantBuffer; }

namespace d14engine::pipeline
{
    struct Sprite : renderer::DrawObject
    {
        // prevent std::unique_ptr from generating default deleter
        virtual ~Sprite() = default;

    protected:
        void onRendererUpdateObjectHelper(renderer::Renderer* rndr) override;
        void onRendererDrawD3d12ObjectHelper(renderer::Renderer* rndr) override;

    public:
        TextureSequence textureData = {};

    public:
        UINT rootParamIndex = 0;

        // Indicates how many buffers need to be updated in the subsequent render passes.
        UINT dirtyFrameCount = renderer::FrameResource::g_bufferCount;

    protected:
        using Buffer = UniquePtr<renderer::ConstantBuffer>;
        using BufferArray = renderer::FrameResource::Array<Buffer>;

        BufferArray m_buffers = {};

    public:
        const BufferArray& buffers() const;
    };
}
