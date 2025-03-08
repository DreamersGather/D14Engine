#pragma once

#include "Common/Precompile.h"

#include "UIKit/MaskObject.h"

namespace d14engine::uikit
{
    struct ShadowMask : MaskObject
    {
        using MaskObject::MaskObject;

        bool enabled = true;

        D2D1_COLOR_F color = D2D1::ColorF{ 0x000000 };

        float standardDeviation = 3.0f;

        D2D1_SHADOW_OPTIMIZATION optimization = D2D1_SHADOW_OPTIMIZATION_BALANCED;

        D2D1_POINT_2F offset = { 0.0f, 0.0f };

        void configEffectInput(ID2D1Effect* effect);
    };
}
