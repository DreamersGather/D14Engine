#pragma once

#include "Common/Precompile.h"

#include "UIKit/BitmapObject.h"

namespace d14engine::uikit
{
    struct MaskObject : BitmapObject
    {
        MaskObject() = default;

        explicit MaskObject(const D2D1_SIZE_F& size);

        MaskObject(float width, float height);

        D2D1_COLOR_F color = D2D1::ColorF{ 0x000000, 0.0f };

        void loadBitmap(const D2D1_SIZE_F& size);

        void loadBitmap(float width, float height);

        void beginDraw(ID2D1DeviceContext* context, const D2D1_MATRIX_3X2_F& transform = D2D1::Matrix3x2F::Identity());

        void endDraw(ID2D1DeviceContext* context);
    };
}
