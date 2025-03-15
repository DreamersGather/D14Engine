#pragma once

#include "Common/Precompile.h"

#include "UIKit/Panel.h"
#include "UIKit/ValuefulObject.h"

namespace d14engine::uikit
{
    struct SliderBase : Panel, ValuefulObject<float>
    {
        SliderBase(
            const D2D1_RECT_F& rect = {},
            float value = 0.0f,
            float minValue = 0.0f,
            float maxValue = 100.0f);

        ////////////////////////
        // Callback Functions //
        ////////////////////////

        //------------------------------------------------------------------
        // Public Interfaces
        //------------------------------------------------------------------
    public:
        void onStartSliding(float value);

        Function<void(SliderBase*, float)> f_onStartSliding = {};

        void onEndSliding(float value);

        Function<void(SliderBase*, float)> f_onEndSliding = {};

        //------------------------------------------------------------------
        // Protected Helpers
        //------------------------------------------------------------------
    protected:
        void onStartSlidingHelper(float value);
        void onEndSlidingHelper(float value);

        ///////////////////////
        // Interaction Logic //
        ///////////////////////

        //------------------------------------------------------------------
        // Slide State
        //------------------------------------------------------------------
    protected:
        bool m_isSliding = false;

        //------------------------------------------------------------------
        // Step Settings
        //------------------------------------------------------------------
    public:
        float stepInterval = 0.0f;

        enum class StepMode
        {
            Continuous, Discrete
        };
        constexpr static auto Continuous = StepMode::Continuous;
        constexpr static auto Discrete = StepMode::Discrete;

        StepMode stepMode = Continuous;

        //------------------------------------------------------------------
        // Value Converter
        //------------------------------------------------------------------
    protected:
        virtual D2D1_POINT_2F valueToOffset(float value) const = 0;
        virtual float offsetToValue(const D2D1_POINT_2F& offset) const = 0;

        void updateValue(const D2D1_POINT_2F& offset);

        //------------------------------------------------------------------
        // Miscellanous
        //------------------------------------------------------------------
    public:
        // If true, a mouse-double-click will also trigger the sliding event.
        bool captureDoubleClick = false;

        /////////////////////////
        // Interface Overrides //
        /////////////////////////

    public:
        //------------------------------------------------------------------
        // ValuefulObject
        //------------------------------------------------------------------

        bool setValue(float value) override;

        bool setMinValue(float value) override;

        bool setMaxValue(float value) override;

    protected:
        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        void onMouseMoveHelper(MouseMoveEvent& e) override;

        void onMouseButtonHelper(MouseButtonEvent& e) override;
    };
}
