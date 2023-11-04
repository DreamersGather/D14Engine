#pragma once

#include "Common/Precompile.h"

#include "Renderer/Interfaces/DrawObject.h"

namespace d14engine::pipeline
{
    struct Sprite : renderer::DrawObject
    {
        explicit Sprite(WstrParam imagePath);

        void onRendererUpdateObjectHelper(renderer::Renderer* rndr) override;
        void onRendererDrawD3d12ObjectHelper(renderer::Renderer* rndr) override;
    };
}
