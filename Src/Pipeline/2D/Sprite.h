#pragma once

#include "Common/Precompile.h"

#include "Renderer/FrameData/FrameAnimation.h"
#include "Renderer/Interfaces/DrawObject.h"

namespace d14engine::renderer { struct DefaultBuffer; }

namespace d14engine::pipeline
{
    struct Sprite : renderer::DrawObject
    {
        explicit Sprite(renderer::Renderer* rndr);

        // prevent std::unique_ptr from generating default deleter
        virtual ~Sprite() = default;

        void onRendererUpdateObjectHelper(renderer::Renderer* rndr) override;
        void onRendererDrawD3d12ObjectHelper(renderer::Renderer* rndr) override;

    protected:
        Renderer* rndr = nullptr;

    public:
        renderer::FrameAnimation<SharedPtr<renderer::DefaultBuffer>> fanim = {};

    public:
        UINT rootParamIndex = 0;

        // Indicates how many buffers need to be updated in the subsequent render passes.
        UINT dirtyFrameCount = FrameResource::g_bufferCount;

    protected:
        using BufferArray = FrameResource::Array<UniquePtr<ConstantBuffer>>;

        BufferArray m_buffers = {};

    public:
        const BufferArray& buffers() const;
    };
}
