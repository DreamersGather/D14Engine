#include "Common/Precompile.h"

#include "UIKit/AnimationUtils/BitmapSequence.h"

using namespace d14engine::renderer;

namespace d14engine::uikit::animation_utils
{
    void BitmapSequence::draw(Renderer* rndr, const D2D1_RECT_F& rect)
    {
        if (visible)
        {
            auto index = fanim.currFrameIndex();
            if (index.has_value())
            {
                auto& f = fanim.frames[index.value()];
                if (f.data)
                {
                    rndr->d2d1DeviceContext()->DrawBitmap
                    (
                    /* bitmap               */ f.data.Get(),
                    /* destinationRectangle */ rect,
                    /* opacity              */ f.opacity,
                    /* interpolationMode    */ f.getInterpolationMode()
                    );
                }
            }
        }
    }
}
