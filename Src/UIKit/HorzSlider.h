#pragma once

#include "Common/Precompile.h"

#include "UIKit/Slider.h"

namespace d14engine::uikit
{
    struct HorzSlider : Slider
    {
        HorzSlider(
            const D2D1_RECT_F& rect = {},
            float value = 0.0f,
            float minValue = 0.0f,
            float maxValue = 100.0f);

        /////////////////////////
        // Interface Overrides //
        /////////////////////////

    protected:
        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        void onSizeHelper(SizeEvent& e) override;

        //------------------------------------------------------------------
        // SliderBase
        //------------------------------------------------------------------

        D2D1_POINT_2F valueToOffset(float value) const override;
        float offsetToValue(const D2D1_POINT_2F& offset) const override;

        //------------------------------------------------------------------
        // Slider
        //------------------------------------------------------------------

        D2D1_RECT_F thumbAreaRect(const D2D1_RECT_F& flatRect) const override;

        D2D1_RECT_F valueLabelMainRectInShadow() const override;

        math_utils::Triangle2D valueLabelSideTriangleInShadow() const override;

        D2D1_RECT_F filledBarAbsoluteRect() const override;

        D2D1_RECT_F completeBarAbsoluteRect() const override;

        D2D1_RECT_F handleAbsoluteRect() const override;

        D2D1_RECT_F valueLabelSelfCoordRect() const override;

        D2D1_RECT_F valueLabelShadowSelfCoordRect() const override;
    };
}
