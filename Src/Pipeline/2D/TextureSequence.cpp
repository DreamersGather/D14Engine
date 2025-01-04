#include "Common/Precompile.h"

#include "Pipeline/2D/TextureSequence.h"

#include "Renderer/GpuBuffer.h"

using namespace d14engine::renderer;

namespace d14engine::pipeline
{
    void TextureSequence::draw(Renderer* rndr, const BufferInfo& info)
    {
        if (visible)
        {
            auto index = fanim.currFrameIndex();
            if (index.has_value())
            {
                auto& f = fanim.frames[index.value()];
                if (f)
                {
                    rndr->cmdList()->SetGraphicsRootConstantBufferView(
                        info.rootParamIndex, info.gpuVirtAddr);
                }
            }
        }
    }
}
