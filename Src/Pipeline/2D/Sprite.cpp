#include "Common/Precompile.h"

#include "Pipeline/2D/Sprite.h"

#include "Common/DirectXError.h"

#include "Renderer/GpuBuffer.h"

using namespace d14engine::renderer;

namespace d14engine::pipeline
{
    void Sprite::onRendererUpdateObjectHelper(Renderer* rndr)
    {
        textureData.update(rndr);
    }

    void Sprite::onRendererDrawD3d12ObjectHelper(Renderer* rndr)
    {
        auto buffer = m_buffers.at(rndr->currFrameIndex())->resource();
        textureData.draw(rndr, { rootParamIndex, buffer->GetGPUVirtualAddress() });
    }

    const Sprite::BufferArray& Sprite::buffers() const
    {
        return m_buffers;
    }
}
