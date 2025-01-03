﻿#include "Common/Precompile.h"

#include "UIKit/AnimationUtils/Sequence.h"

#include "Common/MathUtils/2D.h"

#include "Renderer/Renderer.h"
#include "Renderer/TickTimer.h"

#include "UIKit/BitmapObject.h"

using namespace d14engine::renderer;

namespace d14engine::uikit::animation_utils
{
    void DynamicBitmap::restore()
    {
        fanim.restore();
    }

    void DynamicBitmap::update(Renderer* rndr)
    {
        if (enabled)
        {
            fanim.update((float)rndr->timer()->deltaSecs());
        }
    }

    void DynamicBitmap::draw(Renderer* rndr, const D2D1_RECT_F& rect)
    {
        if (visible)
        {
            auto index = fanim.currFrameIndex();
            if (index.has_value())
            {
                auto& f = fanim.frames[index.value()];
                if (f.bitmap)
                {
                    rndr->d2d1DeviceContext()->DrawBitmap(
                        // round to fit pixel size
                        f.bitmap.Get(), math_utils::roundf(rect),
                        f.opacity, f.getInterpolationMode());
                }
            }
        }
    }
}
