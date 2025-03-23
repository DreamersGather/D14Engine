#include "Common/Precompile.h"

#include "UIKit/SliderBase.h"

#include "Common/MathUtils/Basic.h"
#include "Common/RuntimeError.h"

#include "UIKit/Application.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    SliderBase::SliderBase(
        const D2D1_RECT_F& rect,
        float value,
        float minValue,
        float maxValue)
        :
        Panel(rect, resource_utils::solidColorBrush())
    {
        m_value = value;
        m_minValue = minValue;
        m_maxValue = maxValue;
    }

    void SliderBase::onStartSliding(float value)
    {
        onStartSlidingHelper(value);

        if (f_onStartSliding) f_onStartSliding(this, value);
    }

    void SliderBase::onEndSliding(float value)
    {
        onEndSlidingHelper(value);

        if (f_onEndSliding) f_onEndSliding(this, value);
    }

    void SliderBase::onStartSlidingHelper(float value)
    {
        THROW_IF_NULL(Application::g_app);

        Application::g_app->focusUIObject
        (
            Application::FocusType::Mouse, shared_from_this()
        );
    }

    void SliderBase::onEndSlidingHelper(float value)
    {
        THROW_IF_NULL(Application::g_app);

        Application::g_app->focusUIObject
        (
            Application::FocusType::Mouse, nullptr
        );
    }

    bool SliderBase::setValue(float value)
    {
        // ValuefulObject::setValue rejects the value out of range.
        value = std::clamp(value, m_minValue, m_maxValue);

        bool isValueChanged = ValuefulObject::setValue(value);
        if (isValueChanged) onValueChange(m_value);
        return isValueChanged;
    }

    bool SliderBase::setMinValue(float value)
    {
        bool isValueChanged = ValuefulObject::setMinValue(value);
        if (isValueChanged) onValueChange(m_value);
        return isValueChanged;
    }

    bool SliderBase::setMaxValue(float value)
    {
        bool isValueChanged = ValuefulObject::setMaxValue(value);
        if (isValueChanged) onValueChange(m_value);
        return isValueChanged;
    }

    void SliderBase::updateValue(const D2D1_POINT_2F& offset)
    {
        float tmpValue = offsetToValue(offset);

        if (stepMode == StepMode::Discrete && stepInterval > 0.0f)
        {
            float diffValue = tmpValue - m_value;
            float absDiffValue = std::abs(diffValue);

            // Use FLT_EPSILON to avoid the floating-point error.
            if (absDiffValue + FLT_EPSILON >= stepInterval)
            {
                auto stepCount = math_utils::round(diffValue / stepInterval);
                setValue(m_value + stepCount * stepInterval);
            }
        }
        else setValue(tmpValue);
    }

    void SliderBase::onMouseMoveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseMoveHelper(e);

        if (m_isSliding) updateValue(absoluteToSelfCoord(e.cursorPoint));
    }

    void SliderBase::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Panel::onMouseButtonHelper(e);

        auto& p = e.cursorPoint;

        if (e.state.leftDown() ||
           (captureDoubleClick && e.state.leftDblclk()))
        {
            m_isSliding = true;

            updateValue(absoluteToSelfCoord(p));
            onStartSliding(m_value);
        }
        else if (e.state.leftUp())
        {
            if (m_isSliding)
            {
                m_isSliding = false;

                onEndSliding(m_value);
            }
        }
    }
}
