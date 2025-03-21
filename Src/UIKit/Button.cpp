﻿#include "Common/Precompile.h"

#include "UIKit/Button.h"

#include "Common/CppLangUtils/PointerEquality.h"
#include "Common/MathUtils/2D.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    Button::Button(
        ShrdPtrRefer<IconLabel> content,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        ClickablePanel(rect, resource_utils::solidColorBrush()),
        m_content(content)
    {
        m_takeOverChildrenDrawing = true;

        roundRadiusX = roundRadiusY = roundRadius;
    }

    Button::Button(
        WstrRefer text,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Button(
            IconLabel::uniformLayout(text),
            roundRadius,
            rect) { }

    void Button::onInitializeFinish()
    {
        ClickablePanel::onInitializeFinish();

        if (!m_content)
        {
            m_content = IconLabel::uniformLayout();
        }
        addUIObject(m_content);

        m_content->transform(selfCoordRect());
    }

    void Button::setEnabled(bool value)
    {
        Panel::setEnabled(value);

        m_state = value ? State::Idle : State::Disabled;

        m_content->setEnabled(value);
    }

    const SharedPtr<IconLabel>& Button::content() const
    {
        return m_content;
    }

    void Button::setContent(ShrdPtrRefer<IconLabel> content)
    {
        if (content && !cpp_lang_utils::isMostDerivedEqual(content, m_content))
        {
            removeUIObject(m_content);

            m_content = content;
            addUIObject(m_content);

            m_content->transform(selfCoordRect());
        }
    }

    void Button::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////////
        // Background //
        ////////////////

        auto& bkgn = appearance().background;

        resource_utils::solidColorBrush()->SetColor(bkgn.color);
        resource_utils::solidColorBrush()->SetOpacity(bkgn.opacity);

        ClickablePanel::drawBackground(rndr);

        /////////////
        // Content //
        /////////////

        auto& foreground = appearance().foreground;

        m_content->icon.bitmap.opacity = foreground.opacity;
        m_content->label()->appearance().foreground = foreground;

        if (m_content->isD2d1ObjectVisible())
        {
            m_content->onRendererDrawD2d1Object(rndr);
        }

        /////////////
        // Outline //
        /////////////

        auto& stroke = appearance().stroke;

        resource_utils::solidColorBrush()->SetColor(stroke.color);
        resource_utils::solidColorBrush()->SetOpacity(stroke.opacity);

        auto frame = math_utils::inner(m_absoluteRect, stroke.width);
        D2D1_ROUNDED_RECT outlineRect = { frame, roundRadiusX, roundRadiusY };

        rndr->d2d1DeviceContext()->DrawRoundedRectangle
        (
        /* roundedRect */ outlineRect,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ stroke.width
        );
    }

    bool Button::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isInside(p, m_absoluteRect);
    }

    bool Button::releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj)
    {
        if (cpp_lang_utils::isMostDerivedEqual(m_content, uiobj)) return false;

        return Panel::releaseUIObjectHelper(uiobj);
    }

    void Button::onSizeHelper(SizeEvent& e)
    {
        ClickablePanel::onSizeHelper(e);

        m_content->transform(selfCoordRect());
    }

    void Button::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ClickablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void Button::onMouseEnterHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseEnterHelper(e);

        m_state = State::Hover;
    }

    void Button::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseLeaveHelper(e);

        m_state = State::Idle;
    }

    void Button::onMouseButtonPressHelper(Event& e)
    {
        ClickablePanel::onMouseButtonPressHelper(e);

        if (e.left()) m_state = State::Down;
    }

    void Button::onMouseButtonReleaseHelper(Event& e)
    {
        ClickablePanel::onMouseButtonReleaseHelper(e);

        if (e.left()) m_state = State::Hover;
    }
}
