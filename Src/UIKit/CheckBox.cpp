#include "Common/Precompile.h"

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
        CheckMode mode,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        ClickablePanel(rect, resource_utils::solidColorBrush())
    {
        roundRadiusX = roundRadiusY = roundRadius;

        setCheckMode(mode);
    }

    void CheckBox::onInitializeFinish()
    {
        ClickablePanel::onInitializeFinish();

        checkedIcon.loadStrokeStyle();
    }

    void CheckBox::CheckedIcon::loadStrokeStyle()
    {
        CheckBox* cb = m_master;
        THROW_IF_NULL(cb);

        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;
        auto factory = app->renderer()->d2d1Factory();

        auto prop = D2D1::StrokeStyleProperties
        (
        /* startCap */ D2D1_CAP_STYLE_ROUND,
        /* endCap   */ D2D1_CAP_STYLE_ROUND,
        /* dashCap  */ D2D1_CAP_STYLE_ROUND
        );
        auto& style = strokeStyle;

        THROW_IF_FAILED(factory->CreateStrokeStyle
        (
        /* strokeStyleProperties */ prop,
        /* dashes                */ nullptr,
        /* dashesCount           */ 0,
        /* strokeStyle           */ &style
        ));
    }

    CheckBox::CheckMode CheckBox::checkMode() const
    {
        return m_mode;
    }

    void CheckBox::setCheckMode(CheckMode mode)
    {
        m_mode = mode;

        setCheckStateSilently(Unchecked);
    }

    CheckBox::CheckState CheckBox::checkState() const
    {
        return m_state.activeFlag;
    }

    void CheckBox::setCheckState(CheckState state)
    {
        m_state.activeFlag = state;

        StatefulObject::Event soe = {};
        soe.flag = m_state.activeFlag;

        if (m_stateDetail != soe)
        {
            m_stateDetail = soe;
            onStateChange(m_stateDetail);
        }
    }

    void CheckBox::setCheckStateSilently(CheckState state)
    {
        m_state.activeFlag = state;
        m_stateDetail.flag = m_state.activeFlag;
    }

    const CheckBox::StateMapGroup CheckBox::g_stateMaps =
    {
        StateMap // Binary
        {
            /* Unchecked    */ Checked,
            /* Intermediate */ Unchecked,
            /* Checked      */ Unchecked
        },
        StateMap // TriState
        {
            /* Unchecked    */ Intermediate,
            /* Intermediate */ Checked,
            /* Checked      */ Unchecked
        }
    };
    const CheckBox::StateMap& CheckBox::stateMap() const
    {
        return g_stateMaps[(size_t)m_mode];
    }

    CheckBox::CheckState CheckBox::nextState() const
    {
        return stateMap()[(size_t)checkState()];
    }

    void CheckBox::setEnabled(bool value)
    {
        ClickablePanel::setEnabled(value);

        m_state.buttonFlag = value ?
            State::ButtonFlag::Idle :
            State::ButtonFlag::Disabled;
    }

    void CheckBox::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        auto& setting = appearance().button[m_state.index()];

        ////////////////
        // Background //
        ////////////////

        auto& background = setting.background;

        resource_utils::solidColorBrush()->SetColor(background.color);
        resource_utils::solidColorBrush()->SetOpacity(background.opacity);

        drawBackground(rndr);

        //////////
        // Icon //
        //////////

        switch (checkState())
        {
        case Unchecked:
        {
            break; // There is no foreground icon when unchecked.
        }
        case Intermediate:
        {
            auto& geoSetting = appearance().icon.geometry.intermediate;
            auto& background = appearance().icon.background[m_state.index()];

            resource_utils::solidColorBrush()->SetColor(background.color);
            resource_utils::solidColorBrush()->SetOpacity(background.opacity);

            rndr->d2d1DeviceContext()->FillRectangle
            (
            /* rect  */ math_utils::centered(m_absoluteRect, geoSetting.size),
            /* brush */ resource_utils::solidColorBrush()
            );
            break;
        }
        case Checked:
        {
            auto& geoSetting = appearance().icon.geometry.checked;
            auto& background = appearance().icon.background[m_state.index()];

            resource_utils::solidColorBrush()->SetColor(background.color);
            resource_utils::solidColorBrush()->SetOpacity(background.opacity);

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
            break;
        }
        default: break;
        }
        /////////////
        // Outline //
        /////////////

        auto& stroke = setting.stroke;

        resource_utils::solidColorBrush()->SetColor(stroke.color);
        resource_utils::solidColorBrush()->SetOpacity(stroke.opacity);

        auto rect = math_utils::inner(m_absoluteRect, stroke.width);
        D2D1_ROUNDED_RECT roundedRect = { rect, roundRadiusX, roundRadiusY };

        rndr->d2d1DeviceContext()->DrawRoundedRectangle
        (
        /* roundedRect */ roundedRect,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ stroke.width
        );
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

    void CheckBox::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ClickablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
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

            setCheckState(nextState());
        }
    }
}
