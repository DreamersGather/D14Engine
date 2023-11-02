#include "Common/Precompile.h"

#include "Common/DirectXError.h"

#include "Pipeline/2D/Sprite.h"

#include "Renderer/GraphUtils/Bitmap.h"

using namespace d14engine::renderer;

namespace d14engine::pipeline
{
    Sprite::Sprite(WstrParam imageFile, WstrParam binaryPath)
    {
        auto source = graph_utils::bitmap::load(imageFile, binaryPath);
        auto data = graph_utils::bitmap::map(source.Get());
    }

    void Sprite::onRendererUpdateObjectHelper(Renderer* rndr)
    {
    }

    void Sprite::onRendererDrawD3d12ObjectHelper(Renderer* rndr)
    {
    }
}
