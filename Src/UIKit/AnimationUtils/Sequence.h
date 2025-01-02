#pragma once

#include "Common/Precompile.h"

#include "Renderer/FrameData/FrameAnimation.h"

#include "UIKit/BitmapObject.h"

namespace d14engine::renderer { struct Renderer; }

namespace d14engine::uikit::animation_utils
{
    using FrameAnim = renderer::FrameAnimation<BitmapObject>;

    using FrameArray = FrameAnim::FrameArray;
    using FramePackage = FrameAnim::FramePackage;

    struct DynamicBitmap
    {
        bool enabled = true;
        bool visible = true;

        FrameAnim fanim = {};

    public:
        void restore();

        void update(renderer::Renderer* rndr);
        void draw(renderer::Renderer* rndr, const D2D1_RECT_F& rect);
    };
}
