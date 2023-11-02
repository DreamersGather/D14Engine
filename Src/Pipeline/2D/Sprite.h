#pragma once

#include "Common/Precompile.h"

#include "Renderer/Interfaces/DrawObject.h"

namespace d14engine::pipeline
{
    struct Sprite : renderer::DrawObject
    {
        Sprite(WstrParam imageFile, WstrParam binaryPath = L"");

        void onRendererUpdateObjectHelper(renderer::Renderer* rndr) override;
        void onRendererDrawD3d12ObjectHelper(renderer::Renderer* rndr) override;
    };
}
