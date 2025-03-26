#include "Common/Precompile.h"

#include "UIKit/Button.h"

#include "Common/CppLangUtils/PointerCompare.h"
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
            rect)
    {
        // Here left blank intentionally.
    }

    void Button::onInitializeFinish()
    {
        ClickablePanel::onInitializeFinish();

        if (m_content == nullptr)
        {
            m_content = IconLabel::uniformLayout();
        }
        addUIObject(m_content);

        m_content->transform(selfCoordRect());
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

    void Button::setEnabled(bool value)
    {
        Panel::setEnabled(value);

        m_state = value ? ButtonState::Idle : ButtonState::Disabled;

        m_content->setEnabled(value);
    }

    void Button::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////////
        // Background //
        ////////////////

        auto& background = appearance().background;

        resource_utils::solidColorBrush()->SetColor(background.color);
        resource_utils::solidColorBrush()->SetOpacity(background.opacity);

        drawBackground(rndr);

        /////////////
        // Content //
        /////////////

        auto& foreground = appearance().foreground;

        m_content->icon.bitmap.opacity = foreground.opacity;
        m_content->label()->appearance().foreground = foreground;

        drawChildrenObjects(rndr);

        /////////////
        // Outline //
        /////////////

        auto& stroke = appearance().stroke;

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

    bool Button::releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj)
    {
        if (cpp_lang_utils::isMostDerivedEqual(uiobj, m_content)) return false;

        return ClickablePanel::releaseUIObjectHelper(uiobj);
    }

    bool Button::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isInside(p, m_absoluteRect);
    }

    void Button::onSizeHelper(SizeEvent& e)
    {
        ClickablePanel::onSizeHelper(e);

        m_content->transform(selfCoordRect());
    }

    void Button::onMouseEnterHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseEnterHelper(e);

        m_state = ButtonState::Hover;
    }

    void Button::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ClickablePanel::onMouseLeaveHelper(e);

        m_state = ButtonState::Idle;
    }

    void Button::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ClickablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void Button::onMouseButtonPressHelper(Event& e)
    {
        ClickablePanel::onMouseButtonPressHelper(e);

        if (e.left()) m_state = ButtonState::Down;
    }

    void Button::onMouseButtonReleaseHelper(Event& e)
    {
        ClickablePanel::onMouseButtonReleaseHelper(e);

        if (e.left()) m_state = ButtonState::Hover;
    }
}
