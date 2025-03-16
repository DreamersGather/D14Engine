#include "Common/Precompile.h"

#include "UIKit/TabCaption.h"

#include "Common/CppLangUtils/PointerEquality.h"
#include "Common/MathUtils/2D.h"
#include "Common/RuntimeError.h"

#include "UIKit/Application.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/TabGroup.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    TabCaption::TabCaption(ShrdPtrRefer<IconLabel> title)
        :
        m_title(title) { }

    TabCaption::TabCaption(WstrRefer text)
        :
        TabCaption(IconLabel::compactLayout(text)) { }

    void TabCaption::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        if (!m_title)
        {
            m_title = IconLabel::compactLayout(L"Untitled");
        }
        addUIObject(m_title);

        m_title->transform(titleSelfcoordRect());

        m_title->label()->drawTextOptions = D2D1_DRAW_TEXT_OPTIONS_CLIP;
    }

    void TabCaption::setEnabled(bool value)
    {
        Panel::setEnabled(value);

        m_title->setEnabled(value);
    }

    const SharedPtr<IconLabel>& TabCaption::title() const
    {
        return m_title;
    }

    void TabCaption::setTitle(ShrdPtrRefer<IconLabel> title)
    {
        if (title && !cpp_lang_utils::isMostDerivedEqual(title, m_title))
        {
            removeUIObject(m_title);

            m_title = title;
            addUIObject(m_title);

            m_title->transform(titleSelfcoordRect());
        }
    }

    const WeakPtr<TabGroup>& TabCaption::parentTabGroup() const
    {
        return m_parentTabGroup;
    }

    D2D1_RECT_F TabCaption::titleSelfcoordRect() const
    {
        auto selfRect = selfCoordRect();

        auto& title = appearance().title;
        D2D1_POINT_2F offset =
        {
            title.leftPadding, -title.rightPadding
        };
        return math_utils::increaseLeftRight(selfRect, offset);
    }

    D2D1_POINT_2F TabCaption::closeButtonAbsolutePosition() const
    {
        auto rightTop = math_utils::rightTop(m_absoluteRect);

        auto& buttonGeo = appearance().closeX.button.geometry;
        D2D1_POINT_2F buttonOffset =
        {
            buttonGeo.rightOffset,
            (height() - buttonGeo.size.height) * 0.5f
        };
        auto buttonLeftTop = math_utils::offset(rightTop, buttonOffset);

        return buttonLeftTop;
    }

    D2D1_RECT_F TabCaption::closeIconAbsoluteRect() const
    {
        auto& iconGeo = appearance().closeX.icon.geometry;

        auto iconLeftTop = math_utils::offset
        (
            closeButtonAbsolutePosition(), iconGeo.offset
        );
        return math_utils::rect(iconLeftTop, iconGeo.size);
    }

    D2D1_RECT_F TabCaption::closeButtonAbsoluteRect() const
    {
        return math_utils::rect
        (
            closeButtonAbsolutePosition(),
            appearance().closeX.button.geometry.size
        );
    }

    TabCaption::ButtonState TabCaption::getCloseButtonState() const
    {
        if (m_isCloseButtonDown) return ButtonState::Down;
        else if (m_isCloseButtonHover) return ButtonState::Hover;
        else return ButtonState::Idle;
    }

    void TabCaption::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        if (closable)
        {
            auto stateIndex = (size_t)getCloseButtonState();

            //////////////////
            // Close Button //
            //////////////////

            auto& buttonSetting = appearance().closeX.button;
            auto& buttonBackground = buttonSetting.background[stateIndex];

            resource_utils::solidColorBrush()->SetColor(buttonBackground.color);
            resource_utils::solidColorBrush()->SetOpacity(buttonBackground.opacity);

            D2D1_ROUNDED_RECT buttonRect =
            {
                closeButtonAbsoluteRect(),
                buttonSetting.geometry.roundRadius,
                buttonSetting.geometry.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ buttonRect,
            /* brush       */ resource_utils::solidColorBrush()
            );

            ////////////////
            // Close Icon //
            ////////////////

            auto& iconSetting = appearance().closeX.icon;
            auto& iconBackground = iconSetting.background[stateIndex];

            resource_utils::solidColorBrush()->SetColor(iconBackground.color);
            resource_utils::solidColorBrush()->SetOpacity(iconBackground.opacity);

            auto iconRect = closeIconAbsoluteRect();

            rndr->d2d1DeviceContext()->DrawLine
            (
            /* point0      */ math_utils::leftTop(iconRect),
            /* point1      */ math_utils::rightBottom(iconRect),
            /* brush       */ resource_utils::solidColorBrush(),
            /* strokeWidth */ iconSetting.strokeWidth
            );
            rndr->d2d1DeviceContext()->DrawLine
            (
            /* point0      */ math_utils::rightTop(iconRect),
            /* point1      */ math_utils::leftBottom(iconRect),
            /* brush       */ resource_utils::solidColorBrush(),
            /* strokeWidth */ iconSetting.strokeWidth
            );
        }
    }

    bool TabCaption::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isInside(p, m_absoluteRect);
    }

    void TabCaption::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        m_title->transform(titleSelfcoordRect());
    }

    void TabCaption::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Panel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void TabCaption::onMouseMoveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseMoveHelper(e);

        auto& p = e.cursorPoint;

        if (!math_utils::isOverlapped(p, closeButtonAbsoluteRect()))
        {
            m_isCloseButtonHover = false;
            m_isCloseButtonDown = false;
        }
        else m_isCloseButtonHover = true;
    }

    void TabCaption::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseLeaveHelper(e);

        m_isCloseButtonHover = false;
        m_isCloseButtonDown = false;
    }

    void TabCaption::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Panel::onMouseButtonHelper(e);

        THROW_IF_NULL(Application::g_app);

        if (e.state.leftDown() || e.state.leftDblclk())
        {
            m_isCloseButtonDown = m_isCloseButtonHover;

            if ((!closable || !m_isCloseButtonHover) && !m_parentTabGroup.expired())
            {
                auto tabGroup = m_parentTabGroup.lock();

                size_t tabIndex = 0;
                for (auto& tab : tabGroup->tabs())
                {
                    if (cpp_lang_utils::isMostDerivedEqual(tab.caption, shared_from_this())) break;
                    ++tabIndex;
                }
                tabGroup->selectTab(tabIndex);

                Application::g_app->sendNextImmediateMouseMoveEvent = true;
            }
        }
        else if (e.state.leftUp())
        {
            if (m_isCloseButtonDown)
            {
                m_isCloseButtonDown = false;

                if (closable && !m_parentTabGroup.expired())
                {
                    auto tabGroup = m_parentTabGroup.lock();

                    size_t tabIndex = 0;
                    for (auto& tab : tabGroup->tabs())
                    {
                        if (cpp_lang_utils::isMostDerivedEqual(tab.caption, shared_from_this())) break;
                        ++tabIndex;
                    }
                    tabGroup->removeTab(tabIndex);

                    Application::g_app->sendNextImmediateMouseMoveEvent = true;
                }
            }
        }
    }
}
