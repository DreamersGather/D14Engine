#include "Common/Precompile.h"

#include "UIKit/TextInput.h"

#include "Common/RuntimeError.h"

#include "Renderer/TickTimer.h"

#include "UIKit/AnimationUtils/MotionFunctions.h"
#include "UIKit/Application.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    void TextInput::setEnabled(bool value)
    {
        RawTextInput::setEnabled(value);

        THROW_IF_NULL(Application::g_app);

        if (value)
        {
            m_state = State::Idle;
        }
        else // lose focus when disabled
        {
            if (isFocused())
            {
                Application::g_app->focusUIObject(nullptr);
            }
            m_state = State::Disabled;
        }
    }

    void TextInput::onRendererUpdateObject2DHelper(Renderer* rndr)
    {
        RawTextInput::onRendererUpdateObject2DHelper(rndr);

        auto& animSetting = appearance().bottomLine.animation;

        auto deltaSecs = (float)rndr->timer()->deltaSecs();
        float totalDistance = width() - 2.0f * roundRadiusX;

        if (isFocused() && animSetting.enabled && totalDistance > 0.0f)
        {
            m_dynamicBottomLineLength = animation_utils::motionAccelUniformDecel
                (m_dynamicBottomLineLength, deltaSecs, totalDistance,
                animSetting.durationInSecs.uniform, animSetting.durationInSecs.variable);
        }
    }

    void TextInput::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        auto& srcSetting = appearance().main[(size_t)m_state];
        auto& dstSetting = Label::appearance();

        dstSetting.background = srcSetting.background;
        dstSetting.stroke = srcSetting.stroke;

        RawTextInput::onRendererDrawD2d1LayerHelper(rndr);
    }

    void TextInput::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////////////////
        // Static Bottom Line //
        ////////////////////////

        auto& srcBtlnSetting = appearance().bottomLine;
        auto& dstSetting = RawTextInput::appearance();

        dstSetting.bottomLine.bottomOffset = srcBtlnSetting.bottomOffset;
        dstSetting.bottomLine.background = srcBtlnSetting.background.Static[(size_t)m_state];
        dstSetting.bottomLine.strokeWidth = srcBtlnSetting.strokeWidth;

        RawTextInput::onRendererDrawD2d1ObjectHelper(rndr);

        /////////////////////////
        // Dynamic Bottom Line //
        /////////////////////////

        if (m_dynamicBottomLineLength > 0.0f)
        {
            auto& dynamicBackground = srcBtlnSetting.background.Dynamic;

            resource_utils::solidColorBrush()->SetColor(dynamicBackground.color);
            resource_utils::solidColorBrush()->SetOpacity(dynamicBackground.opacity);

            auto point0 = math_utils::offset(math_utils::leftBottom(m_absoluteRect),
            {
                roundRadiusX, srcBtlnSetting.bottomOffset
            });
            auto point1 = math_utils::offset(point0, { m_dynamicBottomLineLength, 0.0f });

            auto solidColorBrush = resource_utils::solidColorBrush();
            float strokeWidth = srcBtlnSetting.strokeWidth;

            rndr->d2d1DeviceContext()->DrawLine
            (
            /* point0      */ point0,
            /* point1      */ point1,
            /* brush       */ solidColorBrush,
            /* strokeWidth */ strokeWidth
            );
        }
    }

    void TextInput::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        RawTextInput::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void TextInput::onGetFocusHelper()
    {
        RawTextInput::onGetFocusHelper();

        increaseAnimationCount();

        m_state = State::Active;

        if (!appearance().bottomLine.animation.enabled)
        {
            m_dynamicBottomLineLength = width() - 2.0f * roundRadiusX;
        }
    }

    void TextInput::onLoseFocusHelper()
    {
        RawTextInput::onLoseFocusHelper();

        decreaseAnimationCount();

        m_state = State::Idle;

        m_dynamicBottomLineLength = 0.0f;
    }

    void TextInput::onMouseEnterHelper(MouseMoveEvent& e)
    {
        RawTextInput::onMouseEnterHelper(e);

        if (m_state != State::Active)
        {
            m_state = State::Hover;
        }
    }

    void TextInput::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        RawTextInput::onMouseLeaveHelper(e);

        if (m_state != State::Active)
        {
            m_state = State::Idle;
        }
    }
}
