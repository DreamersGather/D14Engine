#include "Common/Precompile.h"

#include "Common/DirectXError.h"

#include "Pipeline/2D/Sprite.h"

#include "Renderer/GraphUtils/Bitmap.h"

using namespace d14engine::renderer;

namespace d14engine::pipeline
{
    Sprite::Sprite(WstrParam imagePath)
    {
        auto source = graph_utils::bitmap::load(imagePath);
        auto data = graph_utils::bitmap::map(source.Get());
    }

    void Sprite::onRendererUpdateObjectHelper(Renderer* rndr)
    {
    }

    void Sprite::onRendererDrawD3d12ObjectHelper(Renderer* rndr)
    {
    }
}
