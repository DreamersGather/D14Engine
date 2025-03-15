#include "Common/Precompile.h"

#include "UIKit/OnOffSwitch.h"

#include "Common/CppLangUtils/FinalAction.h"
#include "Common/MathUtils/2D.h"

#include "Renderer/TickTimer.h"

#include "UIKit/AnimationUtils/MotionFunctions.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    OnOffSwitch::OnOffSwitch(float roundRadius, const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        ClickablePanel(rect, resource_utils::solidColorBrush())
    {
        roundRadiusX = roundRadiusY = roundRadius;

        m_state = { Off, State::ButtonFlag::Idle };
        m_stateDetail.flag = Off;
    }

    void OnOffSwitch::setEnabled(bool value)
    {
        ClickablePanel::setEnabled(value);

        if (m_animationTargetState != State::ActiveFlag::Finished)
        {
            setOnOffState(m_animationTargetState);
        }
        m_state.buttonFlag = value ?
            State::ButtonFlag::Idle :
            State::ButtonFlag::Disabled;
    }

    void OnOffSwitch::setOnOff(State::ActiveFlag flag)
    {
        m_state.activeFlag = flag;

        StatefulObject::Event soe = {};
        soe.flag = m_state.activeFlag;

        if (soe != m_stateDetail)
        {
            m_stateDetail = soe;
            onStateChange(m_stateDetail);
        }
        if (m_animationTargetState != State::ActiveFlag::Finished)
        {
            m_animationTargetState = State::ActiveFlag::Finished;
            decreaseAnimationCount();
        }
    }

    void OnOffSwitch::setOnOffState(State::ActiveFlag flag)
    {
        m_state.activeFlag = flag;
        m_stateDetail.flag = m_state.activeFlag;

        if (m_animationTargetState != State::ActiveFlag::Finished)
        {
            m_animationTargetState = State::ActiveFlag::Finished;
            decreaseAnimationCount();
        }
    }

    void OnOffSwitch::setOnOffWithAnim(State::ActiveFlag flag)
    {
        if (m_animationTargetState == State::ActiveFlag::Finished)
        {
            m_state.activeFlag = flag;
            m_stateDetail.flag = m_state.activeFlag;

            onStateChange(m_stateDetail);

            m_currHandleLeftOffset = 0.0f;
            m_currHandleDisplacement = 0.0f;

            m_animationTargetState = flag;
            increaseAnimationCount();
        }
    }

    D2D1_RECT_F OnOffSwitch::handleAbsoluteRect() const
    {
        auto& geoSetting = appearance().handle.geometry[m_state.index()];

        float leftOffset =
            m_animationTargetState != State::ActiveFlag::Finished ?
            m_currHandleLeftOffset : geoSetting.getLeftOffset(width());

        auto handleRect = math_utils::centered(
            math_utils::leftBorderRect(m_absoluteRect), geoSetting.size);

        float handleLeftOffset = geoSetting.size.width * 0.5f + leftOffset;

        return math_utils::offset(handleRect, { handleLeftOffset, 0.0f });
    }

    void OnOffSwitch::onRendererUpdateObject2DHelper(Renderer* rndr)
    {
        auto deltaSecs = (float)rndr->timer()->deltaSecs();
        auto& animSetting = appearance().handle.animation;

        if (animSetting.enabled && m_animationTargetState != State::ActiveFlag::Finished)
        {
            auto& geoSettingOn = appearance().handle.geometry[(size_t)State::Flag::OnDown];
            auto& geoSettingOff = appearance().handle.geometry[(size_t)State::Flag::OffDown];

            float leftOffsetOn = geoSettingOn.getLeftOffset(width());
            float leftOffsetOff = geoSettingOff.getLeftOffset(width());

            float totalDistance = std::abs(leftOffsetOn - leftOffsetOff);

            m_currHandleDisplacement =
                animation_utils::motionAccelUniformDecel(
                    m_currHandleDisplacement,
                    deltaSecs, totalDistance,
                    animSetting.durationInSecs.uniform,
                    animSetting.durationInSecs.variable);

            auto updateState = cpp_lang_utils::finally([&]
            {
                if (m_currHandleDisplacement >= totalDistance)
                {
                    m_animationTargetState = State::ActiveFlag::Finished;

                    decreaseAnimationCount();
                }
            });
            if (m_animationTargetState == On)
            {
                if (leftOffsetOn >= leftOffsetOff)
                {
                    m_currHandleLeftOffset = leftOffsetOff + m_currHandleDisplacement;
                }
                else m_currHandleLeftOffset = leftOffsetOff - m_currHandleDisplacement;
            }
            else if (m_animationTargetState == Off)
            {
                if (leftOffsetOff >= leftOffsetOn)
                {
                    m_currHandleLeftOffset = leftOffsetOn + m_currHandleDisplacement;
                }
                else m_currHandleLeftOffset = leftOffsetOn - m_currHandleDisplacement;
            }
        }
    }

    void OnOffSwitch::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        auto& setting = appearance().main[m_state.index()];

        ////////////////
        // Background //
        ////////////////

        resource_utils::solidColorBrush()->SetColor(setting.background.color);
        resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

        Panel::drawBackground(rndr);

        ////////////
        // Handle //
        ////////////

        auto& geoSetting = appearance().handle.geometry[m_state.index()];
        auto& bkgnSetting = appearance().handle.background[m_state.index()];

        resource_utils::solidColorBrush()->SetColor(bkgnSetting.color);
        resource_utils::solidColorBrush()->SetOpacity(bkgnSetting.opacity);

        D2D1_ROUNDED_RECT handleRoundedRect =
        {
            handleAbsoluteRect(),
            geoSetting.roundRadius, geoSetting.roundRadius
        };
        rndr->d2d1DeviceContext()->FillRoundedRectangle
        (
        /* roundedRect */ handleRoundedRect,
        /* brush       */ resource_utils::solidColorBrush()
        );

        /////////////
        // Outline //
        /////////////

        resource_utils::solidColorBrush()->SetColor(setting.stroke.color);
        resource_utils::solidColorBrush()->SetOpacity(setting.stroke.opacity);

        auto frame = math_utils::inner(m_absoluteRect, setting.stroke.width);
        D2D1_ROUNDED_RECT outlineRect = { frame, roundRadiusX, roundRadiusY };

        rndr->d2d1DeviceContext()->DrawRoundedRectangle
        (
        /* roundedRect */ outlineRect,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ setting.stroke.width
        );
    }

    void OnOffSwitch::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ClickablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void OnOffSwitch::onMouseEnterHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseEnterHelper(e);

        m_state.buttonFlag = State::ButtonFlag::Hover;
    }

    void OnOffSwitch::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseLeaveHelper(e);

        m_state.buttonFlag = State::ButtonFlag::Idle;
    }

    void OnOffSwitch::onMouseButtonPressHelper(ClickablePanel::Event& e)
    {
        ClickablePanel::onMouseButtonPressHelper(e);

        m_state.buttonFlag = State::ButtonFlag::Down;
    }

    void OnOffSwitch::onMouseButtonReleaseHelper(ClickablePanel::Event& e)
    {
        ClickablePanel::onMouseButtonReleaseHelper(e);

        if (e.left())
        {
            m_state.buttonFlag = State::ButtonFlag::Hover;

            auto targetState = m_stateDetail.on() ? Off : On;

            appearance().handle.animation.enabled ?
                setOnOffWithAnim(targetState) : setOnOff(targetState);
        }
    }
}
