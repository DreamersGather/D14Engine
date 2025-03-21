﻿#include "Common/Precompile.h"

#include "UIKit/CheckBox.h"

#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"

#include "Renderer/Renderer.h"

#include "UIKit/Application.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    CheckBox::CheckBox(
        bool isTripleState,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        ClickablePanel(rect, resource_utils::solidColorBrush())
    {
        roundRadiusX = roundRadiusY = roundRadius;

        m_state = { Unchecked, State::ButtonFlag::Idle };
        m_stateDetail.flag = Unchecked;

        enableTripleState(isTripleState);
    }

    void CheckBox::onInitializeFinish()
    {
        ClickablePanel::onInitializeFinish();

        loadCheckedIconStrokeStyle();
    }

    void CheckBox::loadCheckedIconStrokeStyle()
    {
        THROW_IF_NULL(Application::g_app);

        auto factory = Application::g_app->dx12Renderer()->d2d1Factory();

        auto prop = D2D1::StrokeStyleProperties
        (
        /* startCap */ D2D1_CAP_STYLE_ROUND,
        /* endCap   */ D2D1_CAP_STYLE_ROUND,
        /* dashCap  */ D2D1_CAP_STYLE_ROUND
        );
        auto& style = checkedIcon.strokeStyle;

        THROW_IF_FAILED(factory->CreateStrokeStyle
        (
        /* strokeStyleProperties */ prop,
        /* dashes                */ nullptr,
        /* dashesCount           */ 0,
        /* strokeStyle           */ &style
        ));
    }

    void CheckBox::setEnabled(bool value)
    {
        ClickablePanel::setEnabled(value);

        m_state.buttonFlag = value ?
            State::ButtonFlag::Idle :
            State::ButtonFlag::Disabled;
    }

    void CheckBox::setChecked(State::ActiveFlag flag)
    {
        m_state.activeFlag = flag;

        StatefulObject::Event soe = {};
        soe.flag = m_state.activeFlag;

        if (m_stateDetail != soe)
        {
            m_stateDetail = soe;
            onStateChange(m_stateDetail);
        }
    }

    void CheckBox::setCheckedState(State::ActiveFlag flag)
    {
        m_state.activeFlag = flag;
        m_stateDetail.flag = m_state.activeFlag;
    }

    bool CheckBox::isTripleState() const
    {
        return m_isTripleState;
    }

    void CheckBox::enableTripleState(bool value)
    {
        setChecked(Unchecked);

        m_isTripleState = value;
        if (m_isTripleState)
        {
            m_stateTransitionMap =
            {
            /* Unchecked    */ State::ActiveFlag::Intermediate,
            /* Intermediate */ State::ActiveFlag::Checked,
            /* Checked      */ State::ActiveFlag::Unchecked
            };
        }
        else // Use double-state mode.
        {
            m_stateTransitionMap =
            {
            /* Unchecked    */ State::ActiveFlag::Checked,
            /* Intermediate */ State::ActiveFlag::Unchecked,
            /* Checked      */ State::ActiveFlag::Unchecked
            };
        }
    }

    void CheckBox::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        auto& setting = appearance().button[m_state.index()];

        ////////////////
        // Background //
        ////////////////

        resource_utils::solidColorBrush()->SetColor(setting.background.color);
        resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

        ClickablePanel::drawBackground(rndr);

        //////////
        // Icon //
        //////////

        if (m_state.activeFlag == Intermediate)
        {
            auto& geoSetting = appearance().icon.geometry.intermediate;
            auto& iconBackground = appearance().icon.background[m_state.index()];

            resource_utils::solidColorBrush()->SetColor(iconBackground.color);
            resource_utils::solidColorBrush()->SetOpacity(iconBackground.opacity);

            rndr->d2d1DeviceContext()->FillRectangle
            (
            /* rect  */ math_utils::centered(m_absoluteRect, geoSetting.size),
            /* brush */ resource_utils::solidColorBrush()
            );
        }
        else if (m_state.activeFlag == Checked)
        {
            auto& geoSetting = appearance().icon.geometry.checked;
            auto& iconBackground = appearance().icon.background[m_state.index()];

            resource_utils::solidColorBrush()->SetColor(iconBackground.color);
            resource_utils::solidColorBrush()->SetOpacity(iconBackground.opacity);

            auto iconLeftTop = absolutePosition();

            rndr->d2d1DeviceContext()->DrawLine
            (
            /* point0      */ math_utils::offset(iconLeftTop, geoSetting.tickLine0.point0),
            /* point1      */ math_utils::offset(iconLeftTop, geoSetting.tickLine0.point1),
            /* brush       */ resource_utils::solidColorBrush(),
            /* strokeWidth */ geoSetting.strokeWidth,
            /* strokeStyle */ checkedIcon.strokeStyle.Get()
            );
            rndr->d2d1DeviceContext()->DrawLine
            (
            /* point0      */ math_utils::offset(iconLeftTop, geoSetting.tickLine1.point0),
            /* point1      */ math_utils::offset(iconLeftTop, geoSetting.tickLine1.point1),
            /* brush       */ resource_utils::solidColorBrush(),
            /* strokeWidth */ geoSetting.strokeWidth,
            /* strokeStyle */ checkedIcon.strokeStyle.Get()
            );
        }

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

    void CheckBox::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ClickablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void CheckBox::onMouseEnterHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseEnterHelper(e);

        m_state.buttonFlag = State::ButtonFlag::Hover;
    }

    void CheckBox::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseLeaveHelper(e);

        m_state.buttonFlag = State::ButtonFlag::Idle;
    }

    void CheckBox::onMouseButtonPressHelper(ClickablePanel::Event& e)
    {
        ClickablePanel::onMouseButtonPressHelper(e);

        m_state.buttonFlag = State::ButtonFlag::Down;
    }

    void CheckBox::onMouseButtonReleaseHelper(ClickablePanel::Event& e)
    {
        ClickablePanel::onMouseButtonReleaseHelper(e);

        if (e.left())
        {
            m_state.buttonFlag = State::ButtonFlag::Hover;

            setChecked(m_stateTransitionMap[(size_t)m_state.activeFlag]);
        }
    }
}
