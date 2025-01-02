#pragma once

#include "Common/Precompile.h"

#include "UIKit/AnimationUtils/Sequence.h"
#include "UIKit/Panel.h"

namespace d14engine::uikit
{
    struct FanimPanel : Panel
    {
        using Panel::Panel;

        animation_utils::DynamicBitmap bitmapData = {};

    protected:
        // IDrawObject2D
        void onRendererUpdateObject2DHelper(renderer::Renderer* rndr) override;

        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;
    };
}
