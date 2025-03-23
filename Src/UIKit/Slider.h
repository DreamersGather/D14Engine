#pragma once

#include "Common/Precompile.h"

#include "Common/MathUtils/2D.h"

#include "UIKit/Appearances/Slider.h"
#include "UIKit/ShadowMask.h"
#include "UIKit/SliderBase.h"

namespace d14engine::uikit
{
    struct Label;

    struct Slider : appearance::Slider, SliderBase
    {
        Slider(
            const D2D1_RECT_F& rect = {},
            float value = 0.0f,
            float minValue = 0.0f,
            float maxValue = 100.0f);

        void onInitializeFinish() override;

        _D14_SET_APPEARANCE_PROPERTY(Slider)

        //////////////////////
        // Cached Resources //
        //////////////////////

        using MasterPtr = cpp_lang_utils::EnableMasterPtr<Slider>;

        struct HandleRes : MasterPtr
        {
            using MasterPtr::MasterPtr;

            ShadowMask shadow = {};
            void loadShadow();
        }
        handleRes{ this };

        struct ValueLabelRes : MasterPtr
        {
            using MasterPtr::MasterPtr;

            ShadowMask shadowMask = {};
            Function<void()> loadShadowMask = {};

            MaskObject& mask();
            ShadowMask& shadow();
        }
        valueLabelRes{ this };

        struct SideTriangleRes : MasterPtr
        {
            using MasterPtr::MasterPtr;

            ComPtr<ID2D1PathGeometry> pathGeo = {};
            void loadPathGeo();
        }
        sideTriangleRes{ this };

        /////////////////////////
        // Graphics Components //
        /////////////////////////

        //------------------------------------------------------------------
        // Children Objects
        //------------------------------------------------------------------
    protected:
        SharedPtr<Label> m_valueLabel = {};

    public:
        const SharedPtr<Label>& valueLabel() const;

        //------------------------------------------------------------------
        // Drawing Properties
        //------------------------------------------------------------------
    protected:
        virtual D2D1_RECT_F thumbAreaRect(const D2D1_RECT_F& flatRect) const = 0;

        virtual D2D1_RECT_F valueLabelMainRectInShadow() const = 0;

        virtual math_utils::Triangle2D valueLabelSideTriangleInShadow() const = 0;

        virtual D2D1_RECT_F filledBarAbsoluteRect() const = 0;

        virtual D2D1_RECT_F completeBarAbsoluteRect() const = 0;

        virtual D2D1_RECT_F handleAbsoluteRect() const = 0;

        virtual D2D1_RECT_F valueLabelSelfCoordRect() const = 0;

        virtual D2D1_RECT_F valueLabelShadowSelfCoordRect() const = 0;

        /////////////////////////
        // Interface Overrides //
        /////////////////////////

    protected:
        //------------------------------------------------------------------
        // IDrawObject2D
        //------------------------------------------------------------------

        void onRendererDrawD2d1LayerHelper(renderer::Renderer* rndr) override;

        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        bool isHitHelper(const Event::Point& p) const override;

        void onSizeHelper(SizeEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        //------------------------------------------------------------------
        // ValuefulObject
        //------------------------------------------------------------------

        void onValueChangeHelper(float value) override;

        //------------------------------------------------------------------
        // SliderBase
        //------------------------------------------------------------------

        void onStartSlidingHelper(float value) override;

        void onEndSlidingHelper(float value) override;
    };
}
