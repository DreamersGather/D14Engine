#include "Common/Precompile.h"

#include "UIKit/PopupMenu.h"

#include "Common/RuntimeError.h"

#include "UIKit/Application.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    PopupMenu::PopupMenu(const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        WaterfallView(rect),
        shadow(extendedSize(size()))
    {
        setPrivateVisible(false);
        setPrivateEnabled(false);

        selectMode = SelectMode::None;
    }

    D2D1_SIZE_F PopupMenu::extendedSize(const D2D1_SIZE_F& rawSize) const
    {
        return math_utils::increaseHeight(rawSize, +2.0f * appearance().geometry.extension);
    }

    D2D1_SIZE_F PopupMenu::narrowedSize(const D2D1_SIZE_F& rawSize) const
    {
        return math_utils::increaseHeight(rawSize, -2.0f * appearance().geometry.extension);
    }

    void PopupMenu::onChangeActivity(bool value)
    {
        onChangeActivityHelper(value);

        if (f_onChangeActivity) f_onChangeActivity(this, value);
    }

    void PopupMenu::onTriggerMenuItem(ItemIndexParam itemIndex)
    {
        onTriggerMenuItemHelper(itemIndex);

        if (f_onTriggerMenuItem) f_onTriggerMenuItem(this, itemIndex);
    }

    void PopupMenu::onChangeActivityHelper(bool value)
    {
        THROW_IF_NULL(Application::g_app);

        Application::g_app->sendNextImmediateMouseMoveEvent = true;
    }

    void PopupMenu::onTriggerMenuItemHelper(ItemIndexParam itemIndex)
    {
        // This method intentionally left blank.
    }

    void PopupMenu::insertItem(const ItemList& items, size_t index)
    {
        for (auto& item : items)
        {
            item->m_parentMenu = std::dynamic_pointer_cast<PopupMenu>(shared_from_this());
        }
        WaterfallView::insertItem(items, index);
    }

    void PopupMenu::appendItem(const ItemList& items)
    {
        insertItem(items, m_items.size());
    }

    void PopupMenu::removeItem(size_t index, size_t count)
    {
        if (index >= 0 && index < m_items.size() && count > 0)
        {
            count = std::min(count, m_items.size() - index);
            size_t endIndex = index + count - 1;

            for (ItemIndex itemIndex = { &m_items, index }; itemIndex < endIndex; ++itemIndex)
            {
                (*itemIndex)->m_parentMenu.reset();
            }
        }
        WaterfallView::removeItem(index, count);
    }

    void PopupMenu::clearAllItems()
    {
        for (auto& item : m_items)
        {
            item->m_parentMenu.reset();
        }
        WaterfallView::clearAllItems();
    }

    const WeakPtr<MenuItem>& PopupMenu::associatedItem() const
    {
        return m_associatedItem;
    }

    void PopupMenu::setActivated(bool value)
    {
        setPrivateVisible(value);
        setPrivateEnabled(value);

        if (m_backgroundTriggerPanel)
        {
            m_backgroundTriggerPanel->setPrivateEnabled(value);
            if (value) m_backgroundTriggerPanel->moveTopmost();
        }
        if (value)
        {
            // Place the menu above the bkgn-trigger-panel.
            moveTopmost();
        }
        else if (m_lastHoverItemIndex.valid())
        {
            (*m_lastHoverItemIndex)->triggerLeaveStateTrans();
            m_lastHoverItemIndex.invalidate();
        }
        onChangeActivity(value);
    }

    void PopupMenu::setActivatedIncludingParents(bool value)
    {
        setActivated(value);

        if (!m_associatedItem.expired() && !m_associatedItem.lock()->m_parentMenu.expired())
        {
            m_associatedItem.lock()->m_parentMenu.lock()->setActivatedIncludingParents(value);
        }
    }

    void PopupMenu::setActivatedIncludingChildren(bool value)
    {
        if (m_lastHoverItemIndex.valid() && (*m_lastHoverItemIndex)->m_associatedMenu != nullptr)
        {
            (*m_lastHoverItemIndex)->m_associatedMenu->setActivatedIncludingChildren(value);
        }
        setActivated(value); // Note m_lastHoverItemIndex may be invalidated in setActivated.
    }

    bool PopupMenu::isHorzConstraintMeet(float expectedHorzOffset)
    {
        return expectedHorzOffset + extendedSize(size()).width <= constrainedRectangle.right;
    }

    bool PopupMenu::isVertConstraintMeet(float expectedVertOffset)
    {
        return expectedVertOffset + extendedSize(size()).height <= constrainedRectangle.bottom;
    }

    void PopupMenu::showInConstrainedRect(D2D1_POINT_2F expectedOffset)
    {
        auto extSize = extendedSize(size());

        if (!isHorzConstraintMeet(expectedOffset.x))
        {
            expectedOffset.x = constrainedRectangle.right - extSize.width;
        }
        expectedOffset.x = std::max(expectedOffset.x, constrainedRectangle.left);

        if (!isVertConstraintMeet(expectedOffset.y))
        {
            expectedOffset.y = std::min(expectedOffset.y, constrainedRectangle.bottom) - extSize.height;
        }
        expectedOffset.y = std::max(expectedOffset.y, constrainedRectangle.top);

        move(math_utils::increaseY(expectedOffset, appearance().geometry.extension));
        setActivated(true); // Note a vast menu could still overflow in both directions.
    }

    WeakPtr<Panel> PopupMenu::backgroundTriggerPanel() const
    {
        return m_backgroundTriggerPanel;
    }

    void PopupMenu::setBackgroundTriggerPanel(bool value)
    {
        if (value)
        {
            auto rect = math_utils::infiniteRectF();
            m_backgroundTriggerPanel = makeRootUIObject<Panel>(rect);

            m_backgroundTriggerPanel->setPrivateVisible(false);
            m_backgroundTriggerPanel->setPrivateEnabled(false);

            m_backgroundTriggerPanel->f_onMouseButton = [this]
            (Panel* p, MouseButtonEvent& e)
            {
                setActivatedIncludingChildren(false);
            };
        }
        else // release the background trigger panel
        {
            if (m_backgroundTriggerPanel && m_backgroundTriggerPanel->release())
            {
                m_backgroundTriggerPanel.reset();
            }
        }
    }

    void PopupMenu::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        WaterfallView::onRendererDrawD2d1LayerHelper(rndr);

        /////////////////////
        // Shape of Shadow //
        /////////////////////

        shadow.beginDraw(rndr->d2d1DeviceContext());
        {
            auto& geoSetting = appearance().geometry;
            auto& shadowSetting = appearance().shadow;

            resource_utils::solidColorBrush()->SetOpacity(1.0f);

            auto extRect = math_utils::rect(
                { 0.0f, 0.0f }, extendedSize(size()));

            D2D1_ROUNDED_RECT shadowRect =
            {
                math_utils::moveVertex(extRect, shadowSetting.offset),
                geoSetting.roundRadius, geoSetting.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ shadowRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        shadow.endDraw(rndr->d2d1DeviceContext());
    }

    void PopupMenu::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////
        // Shadow //
        ////////////

        auto& geoSetting = appearance().geometry;
        auto& shadowSetting = appearance().shadow;

        shadow.color = shadowSetting.color;
        shadow.standardDeviation = shadowSetting.standardDeviation;

        shadow.configEffectInput(resource_utils::shadowEffect());

        auto leftTop = absolutePosition();
        auto shadowLeftTop = math_utils::increaseY(leftTop, -geoSetting.extension);

        rndr->d2d1DeviceContext()->DrawImage
        (
        /* effect       */ resource_utils::shadowEffect(),
        /* targetOffset */ shadowLeftTop
        );

        ///////////////
        // Extension //
        ///////////////

        auto& extBkgn = appearance().background;

        resource_utils::solidColorBrush()->SetColor(extBkgn.color);
        resource_utils::solidColorBrush()->SetOpacity(extBkgn.opacity);

        D2D1_ROUNDED_RECT extRect =
        {
            math_utils::stretch(m_absoluteRect, { 0.0f, geoSetting.extension }),
            geoSetting.roundRadius, geoSetting.roundRadius
        };
        rndr->d2d1DeviceContext()->FillRoundedRectangle
        (
        /* roundedRect */ extRect,
        /* brush       */ resource_utils::solidColorBrush()
        );

        //////////////////
        // Body Content //
        //////////////////

        WaterfallView::onRendererDrawD2d1ObjectHelper(rndr);
    }

    void PopupMenu::onSizeHelper(SizeEvent& e)
    {
        WaterfallView::onSizeHelper(e);

        shadow.loadBitmap(extendedSize(e.size));
    }

    void PopupMenu::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        WaterfallView::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void PopupMenu::onMouseMoveHelper(MouseMoveEvent& e)
    {
        ItemIndex lastHoverItemIndex = m_lastHoverItemIndex;

        // Note this callback will update m_lastHoverItemIndex.
        WaterfallView::onMouseMoveHelper(e);

        ItemIndex& currHoverItemIndex = m_lastHoverItemIndex;

        if (lastHoverItemIndex.valid() && lastHoverItemIndex != currHoverItemIndex)
        {
            if ((*lastHoverItemIndex)->m_associatedMenu != nullptr)
            {
                (*lastHoverItemIndex)->m_associatedMenu->setActivated(false);
            }
        }
        if (currHoverItemIndex.valid() && currHoverItemIndex != lastHoverItemIndex)
        {
            if ((*currHoverItemIndex)->m_associatedMenu != nullptr)
            {
                (*currHoverItemIndex)->popupAssociatedMenu();
            }
        }
    }

    void PopupMenu::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ScrollView::onMouseLeaveHelper(e);

        if (m_lastHoverItemIndex.valid())
        {
            if ((*m_lastHoverItemIndex)->m_associatedMenu == nullptr)
            {
                (*m_lastHoverItemIndex)->triggerLeaveStateTrans();
                m_lastHoverItemIndex.invalidate();
            }
        }
    }

    void PopupMenu::onMouseButtonHelper(MouseButtonEvent& e)
    {
        WaterfallView::onMouseButtonHelper(e);

        // In case trigger by mistake when controlling the scroll bars.
        if (e.state.leftUp() && !isControllingScrollBars())
        {
            auto itemIndex = viewportOffsetToItemIndex(
                m_viewportOffset.y + absoluteToSelfCoord(e.cursorPoint).y);

            if (itemIndex.valid())
            {
                if ((*itemIndex)->m_enabled && (*itemIndex)->isTriggerItem)
                {
                    onTriggerMenuItem(itemIndex);
                }
            }
        }
    }
}
