#include "Common/Precompile.h"

#include "UIKit/TabGroup.h"

#include "Common/CppLangUtils/PointerCompare.h"
#include "Common/DirectXError.h"

#include "UIKit/Application.h"
#include "UIKit/PopupMenu.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/TabCaption.h"
#include "UIKit/Window.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    TabGroup::TabGroup(const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        ResizablePanel(rect, resource_utils::solidColorBrush())
    {
        setResizable(false);

        transform(math_utils::adaptMaxSize(rect, minimalSize()));

        m_previewPanel = makeRootUIObject<PopupMenu>();
        m_previewPanel->setBackgroundTriggerPanel(true);
    }

    TabGroup::~TabGroup()
    {
        // No need to do the clearing if the application already destroyed.
        if (Application::g_app != nullptr) m_previewPanel->release();
    }

    void TabGroup::onInitializeFinish()
    {
        ResizablePanel::onInitializeFinish();

        activeCard.loadMask();
        activeCard.loadPathGeo();

        moreCards.loadMask();
        moreCards.loadPathGeo();
    }

    void TabGroup::ActiveCard::loadMask()
    {
        TabGroup* tg = m_master;
        THROW_IF_NULL(tg);

        auto& setting = tg->appearance().tabBar.card.main[(size_t)CardState::Active];

        mask.loadBitmap(setting.geometry.size);
    }

    void TabGroup::ActiveCard::loadPathGeo()
    {
        TabGroup* tg = m_master;
        THROW_IF_NULL(tg);

        THROW_IF_NULL(Application::g_app);

        auto factory = Application::g_app->dx12Renderer()->d2d1Factory();
        THROW_IF_FAILED(factory->CreatePathGeometry(&pathGeo));

        auto& setting = tg->appearance().tabBar.card.main[(size_t)CardState::Active];

        float cardWidth = setting.geometry.size.width;
        float cardHeight = setting.geometry.size.height;

        float cardRoundRadius = setting.geometry.roundRadius;
        D2D1_SIZE_F cardCornerSize = { cardRoundRadius, cardRoundRadius };

        ComPtr<ID2D1GeometrySink> geoSink = {};
        THROW_IF_FAILED(pathGeo->Open(&geoSink));
        {
            geoSink->BeginFigure({ 0.0f, cardHeight }, D2D1_FIGURE_BEGIN_FILLED);

            ////////////////////////
            // Left Bottom Corner //
            ////////////////////////

            geoSink->AddArc(
            {
            /* point            */ { cardRoundRadius, cardHeight - cardRoundRadius },
            /* size             */ cardCornerSize,
            /* rotation degrees */ 90.0f,
            /* sweep direction  */ D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            /* arc size         */ D2D1_ARC_SIZE_SMALL
            });
            geoSink->AddLine({ cardRoundRadius, cardRoundRadius });

            /////////////////////
            // Left Top Corner //
            /////////////////////

            geoSink->AddArc(
            {
            /* point            */ { cardRoundRadius * 2.0f, 0.0f },
            /* size             */ cardCornerSize,
            /* rotation degrees */ 90.0f,
            /* sweep direction  */ D2D1_SWEEP_DIRECTION_CLOCKWISE,
            /* arc size         */ D2D1_ARC_SIZE_SMALL
            });
            geoSink->AddLine({ cardWidth -  cardRoundRadius * 2.0f, 0.0f });

            //////////////////////
            // Right Top Corner //
            //////////////////////

            geoSink->AddArc(
            {
            /* point            */ { cardWidth - cardRoundRadius, cardRoundRadius },
            /* size             */ cardCornerSize,
            /* rotation degrees */ 90.0f,
            /* sweep direction  */ D2D1_SWEEP_DIRECTION_CLOCKWISE,
            /* arc size         */ D2D1_ARC_SIZE_SMALL
            });
            geoSink->AddLine({ cardWidth - cardRoundRadius, cardHeight - cardRoundRadius });

            /////////////////////////
            // Right Bottom Corner //
            /////////////////////////

            geoSink->AddArc(
            {
            /* point            */ { cardWidth, cardHeight },
            /* size             */ cardCornerSize,
            /* rotation degrees */ 90.0f,
            /* sweep direction  */ D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
            /* arc size         */ D2D1_ARC_SIZE_SMALL
            });
            geoSink->AddLine({ 0.0f, cardHeight });

            geoSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        }
        THROW_IF_FAILED(geoSink->Close());
    }

    void TabGroup::MoreCards::loadMask()
    {
        TabGroup* tg = m_master;
        THROW_IF_NULL(tg);

        auto& setting = tg->appearance().tabBar.moreCards.control.button;

        mask.loadBitmap(setting.geometry.size);
    }

    void TabGroup::MoreCards::loadPathGeo()
    {
        TabGroup* tg = m_master;
        THROW_IF_NULL(tg);

        THROW_IF_NULL(Application::g_app);

        auto factory = Application::g_app->dx12Renderer()->d2d1Factory();
        THROW_IF_FAILED(factory->CreatePathGeometry(&pathGeo));

        ComPtr<ID2D1GeometrySink> geoSink = {};
        THROW_IF_FAILED(pathGeo->Open(&geoSink));
        {
            auto& setting = tg->appearance().tabBar.moreCards.control.icon;
            auto& triangleVertices = setting.geometry.bottomTriangle.points;

            geoSink->BeginFigure(triangleVertices[0], D2D1_FIGURE_BEGIN_FILLED);

            geoSink->AddLines(triangleVertices, _countof(triangleVertices));

            geoSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        }
        THROW_IF_FAILED(geoSink->Close());
    }

    D2D1_RECT_F TabGroup::cardBarExtendedAbsoluteRect() const
    {
        return math_utils::increaseTop(
            m_absoluteRect,
            -appearance().tabBar.geometry.height);
    }

    D2D1_RECT_F TabGroup::cardBarExtendedCardBarAbsoluteRect() const
    {
        auto& setting = appearance().tabBar.geometry;

        float top = m_absoluteRect.top - setting.height;
        return math_utils::rect(
            m_absoluteRect.left, top,
            width(), setting.height);
    }

    float TabGroup::minimalWidth() const
    {
        float minWidth = appearance().tabBar.geometry.rightPadding;

        if (m_activeCardTabIndex.valid())
        {
            auto rect = cardAbsoluteRect(m_activeCardTabIndex);
            minWidth += absoluteToSelfCoord(rect).right;
        }
        return minWidth;
    }

    TabGroup::TabIndex TabGroup::selfcoordOffsetToCardTabIndex(float offset) const
    {
        float cardLength = 0.0f;
        for (TabIndex tabIndex = { (TabList*)&m_tabs, 0 };
             tabIndex < m_candidateTabCount; ++tabIndex)
        {
            auto state = getCardState(tabIndex);
            auto& setting = appearance().tabBar.card.main[(size_t)state];

            cardLength += setting.geometry.size.width;
            if (cardLength > offset) return tabIndex;
        }
        return TabIndex{};
    }

    void TabGroup::onSelectedTabIndexChange(TabIndexParam index)
    {
        onSelectedTabIndexChangeHelper(index);

        if (f_onSelectedTabIndexChange) f_onSelectedTabIndexChange(this, index);
    }

    void TabGroup::onSelectedTabIndexChangeHelper(TabIndexParam index)
    {
        // This method intentionally left blank.
    }

    const TabGroup::TabList& TabGroup::tabs() const
    {
        return m_tabs;
    }

    void TabGroup::insertTab(const Tab& tab, size_t index)
    {
        if (!tab.caption || !tab.content) return;

        // "index == m_tabs.size()" ---> append
        index = std::clamp(index, 0_uz, m_tabs.size());

        insertTab(tab, { &m_tabs, index });
    }

    void TabGroup::appendTab(const Tab& tab)
    {
        insertTab(tab, m_tabs.size());
    }

    void TabGroup::removeTab(size_t index, size_t count)
    {
        if (index >= 0 && index < m_tabs.size() && count > 0)
        {
            count = std::min(count, m_tabs.size() - index);

            removeTab({ &m_tabs, index }, count);
        }
    }

    void TabGroup::clearAllTabs()
    {
        // There is no trivial clearing since we need to update each tab-caption.
        removeTab(0, m_tabs.size());
    }

    void TabGroup::selectTab(size_t index)
    {
        if (index >= 0 && index < m_tabs.size())
        {
            selectTab({ &m_tabs, index });
        }
        else if (m_activeCardTabIndex.valid())
        {
            m_activeCardTabIndex->content->setPrivateEnabled(false);
            m_activeCardTabIndex.invalidate();

            updateCandidateTabInfo();
            updatePreviewPanelItems();
        }
    }

    void TabGroup::swapTab(size_t index1, size_t index2)
    {
        if (index1 >= 0 && index1 < m_tabs.size() &&
            index2 >= 0 && index2 < m_tabs.size() && index1 != index2)
        {
            TabIndex tabIndex1, tabIndex2;
            for (TabIndex tabIndex = { &m_tabs, 0 }; tabIndex.valid(); ++tabIndex)
            {
                if (tabIndex.index == index1) tabIndex1 = tabIndex;
                if (tabIndex.index == index2) tabIndex2 = tabIndex;

                if (tabIndex1.valid() && tabIndex2.valid()) break; // both found
            }
            swapTab(tabIndex1, tabIndex2);
        }
    }

    void TabGroup::insertTab(const Tab& tab, TabIndexParam tabIndex)
    {
        if (!tab.caption || !tab.content) return;

        registerUIEvents(tab.content);

        tab.caption->m_parentTabGroup = std::dynamic_pointer_cast<TabGroup>(shared_from_this());

        tab.content->setPrivateEnabled(false);
        tab.content->transform(selfCoordRect());

        auto tabItor = m_tabs.insert(tabIndex.iterator, tab);

        tabItor->m_previewItem = makeUIObject<MenuItem>(nullUIObj());
        tabItor->m_previewItem->isInstant = false;

#define UPDATE_TAB_INDEX(Tab_Index) \
do { \
    if (Tab_Index.generalValid()) \
    { \
        if (insertedIndex <= Tab_Index) \
        { \
            Tab_Index.moveIndexNext(); \
        } \
    } \
} while (0)
        // Note tabIndex may be invalidated if it is one of the following.
        size_t insertedIndex = tabIndex.index;

        UPDATE_TAB_INDEX(m_hoverCardTabIndex);
        UPDATE_TAB_INDEX(m_activeCardTabIndex);
        UPDATE_TAB_INDEX(m_draggedCardTabIndex);

#undef UPDATE_TAB_INDEX

        updateCandidateTabInfo();
        if (tabIndex < m_candidateTabCount)
        {
            registerUIEvents(tabItor->caption);
        }
        else // not a candidate tab
        {
            tabItor->m_previewItem->setContent(tabItor->caption);
        }
        updatePreviewPanelItems();
    }

    void TabGroup::removeTab(TabIndexParam tabIndex, size_t count)
    {
        size_t orgActiveCardIndex = m_activeCardTabIndex.index;

        TabList::iterator tabItor = tabIndex.iterator;

        for (size_t i = 0; i < count; ++i)
        {
            // Its parent could be either tab-group or preview-menu,
            // so using release here ensures that it is properly cleaned up.
            tabItor->caption->release();

            unregisterUIEvents(tabItor->content);
            tabItor->m_previewItem->release();

            tabItor->caption->m_parentTabGroup.reset();

            tabItor = m_tabs.erase(tabItor);
        }
        size_t endIndex = tabIndex.index + count;

#define UPDATE_TAB_INDEX(Tab_Index) \
do { \
    if (Tab_Index.generalValid()) \
    { \
        if (Tab_Index >= endIndex) \
        { \
            Tab_Index.moveIndexPrev(count); \
        } \
        else if (Tab_Index >= removedIndex) \
        { \
            Tab_Index.invalidate(); \
        } \
    } \
} while (0)
        // Note tabIndex may be invalidated if it is one of the following.
        size_t removedIndex = tabIndex.index;

        UPDATE_TAB_INDEX(m_hoverCardTabIndex);
        UPDATE_TAB_INDEX(m_activeCardTabIndex);
        UPDATE_TAB_INDEX(m_draggedCardTabIndex);

#undef UPDATE_TAB_INDEX

        updateCandidateTabInfo();

        if (m_activeCardTabIndex != orgActiveCardIndex)
        {
            onSelectedTabIndexChange(m_activeCardTabIndex);
        }
        updatePreviewPanelItems();
    }

    void TabGroup::selectTab(TabIndexParam tabIndex)
    {
        size_t orgActiveCardIndex = m_activeCardTabIndex.index;

        if (tabIndex != m_activeCardTabIndex)
        {
            if (m_activeCardTabIndex.valid())
            {
                m_activeCardTabIndex->content->setPrivateEnabled(false);
            }
        }
        if (tabIndex >= m_candidateTabCount)
        {
            if (!m_activeCardTabIndex.valid())
            {
                m_activeCardTabIndex = TabIndex::begin(&m_tabs);
            }
            swapTab(tabIndex, m_activeCardTabIndex);
        }
        else m_activeCardTabIndex = tabIndex;

        updateCandidateTabInfo();

        if (m_candidateTabCount > 0)
        {
            m_activeCardTabIndex->content->setPrivateEnabled(true);
        }
        else m_activeCardTabIndex.invalidate();

        if (m_activeCardTabIndex.valid())
        {
            m_activeCardTabIndex->content->transform(selfCoordRect());
        }
        if (m_activeCardTabIndex != orgActiveCardIndex)
        {
            onSelectedTabIndexChange(m_activeCardTabIndex);
        }
        updatePreviewPanelItems();
    }

    void TabGroup::swapTab(TabIndexParam tabIndex1, TabIndexParam tabIndex2)
    {
        if (tabIndex1 != tabIndex2)
        {
            std::swap(*tabIndex1.iterator, *tabIndex2.iterator);

            if (m_activeCardTabIndex == tabIndex1 ||
                m_activeCardTabIndex == tabIndex2)
            {
                onSelectedTabIndexChange(m_activeCardTabIndex);
            }
        }
    }

    TabGroup::CardState TabGroup::getCardState(TabIndexParam tabIndex) const
    {
        if (tabIndex.valid())
        {
            if (tabIndex == m_activeCardTabIndex)
            {
                return CardState::Active;
            }
            else if (tabIndex == m_hoverCardTabIndex)
            {
                return CardState::Hover;
            }
            // fall through
        }
        return CardState::Dormant;
    }

    const TabGroup::TabIndex& TabGroup::activeCardTabIndex() const
    {
        return m_activeCardTabIndex;
    }

    TabGroup::ButtonState TabGroup::getMoreCardsButtonState() const
    {
        if (m_isMoreCardsButtonDown)
        {
            return ButtonState::Down;
        }
        else if (m_isMoreCardsButtonHover)
        {
            return ButtonState::Hover;
        }
        else return ButtonState::Idle;
    }

    const SharedPtr<PopupMenu>& TabGroup::previewPanel() const
    {
        return m_previewPanel;
    }

    void TabGroup::updateCandidateTabInfo()
    {
        TabIndex orgIndex = { &m_tabs, 0 };
        for (; orgIndex < m_candidateTabCount && orgIndex.valid(); ++orgIndex)
        {
            orgIndex->caption->release();
            orgIndex->m_previewItem->setContent(orgIndex->caption);
        }
        m_candidateTabCount = m_tabs.size();

        float cardLength = 0.0f;
        float maxCardLegnth = width() - appearance().tabBar.geometry.rightPadding;

        for (TabIndex tabIndex = { &m_tabs, 0 }; tabIndex.valid(); ++tabIndex)
        {
            auto state = getCardState(tabIndex);
            auto& setting = appearance().tabBar.card.main[(size_t)state];

            float temporaryLength = cardLength + setting.geometry.size.width;
            if (temporaryLength > maxCardLegnth)
            {
                m_candidateTabCount = tabIndex.index;
                break;
            }
            tabIndex->m_cardAbsoluteRectCache =
            {
                m_absoluteRect.left + cardLength,
                m_absoluteRect.top - setting.geometry.size.height,
                m_absoluteRect.left + temporaryLength,
                m_absoluteRect.top + ((state == CardState::Active) ? 0.0f : setting.geometry.roundRadius)
            };
            cardLength = temporaryLength;

            tabIndex->caption->release();
            registerUIEvents(tabIndex->caption);

            // The tab-caption may be disabled when the preview-panel
            // calls updateItemIndexRangeActivity() to optimize performance.
            tabIndex->caption->setPrivateEnabled(true);

            tabIndex->caption->transform(absoluteToSelfCoord(cardCaptionAbsoluteRect(tabIndex)));
        }
    }

    void TabGroup::updatePreviewPanelItems()
    {
        D2D1_POINT_2F orgViewportOffset = m_previewPanel->viewportOffset();

        TabIndex tabIndex = { &m_tabs, 0 };
        for ( ; tabIndex < m_candidateTabCount; ++tabIndex )
        {
            tabIndex->caption->release();
            registerUIEvents(tabIndex->caption);

            // The tab-caption may be disabled when the preview-panel
            // calls updateItemIndexRangeActivity() to optimize performance.
            tabIndex->caption->setPrivateEnabled(true);
        }
        m_previewPanel->clearAllItems();

        auto& cardSetting = appearance().tabBar.card;
        auto& prvwSrc = appearance().tabBar.moreCards.previewPanel;

        auto& prvwDst = m_previewPanel->appearance();
        // LoadShadowBitmap will be called when resizing the preview-panel.
        prvwDst.geometry = prvwSrc.geometry;
        prvwDst.shadow.offset = prvwSrc.shadow.offset;
        prvwDst.shadow.standardDeviation = prvwSrc.shadow.standardDeviation;

        PopupMenu::ItemList prvwItems = {};

        // Insert the non-candidate tabs into the preview-panel.
        for ( ; tabIndex.valid(); ++tabIndex )
        {
            tabIndex->m_previewItem->transform(math_utils::heightOnlyRect(prvwSrc.itemHeight));
            tabIndex->caption->release();
            tabIndex->m_previewItem->setContent(tabIndex->caption);
            tabIndex->m_previewItem->state = ViewItem::State::Idle;

            prvwItems.push_back(tabIndex->m_previewItem);
        }
        m_previewPanel->insertItem(prvwItems);

        float prvwWidth = cardSetting.main[(size_t)CardState::Dormant].geometry.size.width;
        float prvwHeight = (float)(m_tabs.size() - m_candidateTabCount) * prvwSrc.itemHeight;

        // Keep the preview-panel is within the area of the tab-group.
        auto prvwSize = m_previewPanel->extendedSize({ prvwWidth, prvwHeight });
        if (prvwSize.height > height())
        {
            m_previewPanel->setSize(m_previewPanel->narrowedSize({ prvwSize.width, height() }));
        }
        else m_previewPanel->setSize({ prvwWidth, prvwHeight });

        m_previewPanel->setPosition(math_utils::offset(
            math_utils::rightTop(m_absoluteRect),
            math_utils::increaseX(prvwSrc.offset, -m_previewPanel->width())));

        m_previewPanel->setViewportOffset(orgViewportOffset);
    }

    D2D1_RECT_F TabGroup::cardBarAbsoluteRect() const
    {
        auto& setting = appearance().tabBar.card.main[(size_t)CardState::Dormant];
        return
        {
            m_absoluteRect.left,
            m_absoluteRect.top - setting.geometry.size.height,
            m_absoluteRect.right,
            m_absoluteRect.top + setting.geometry.roundRadius
        };
    }

    D2D1_RECT_F TabGroup::cardAbsoluteRect(TabIndexParam tabIndex) const
    {
        return tabIndex.valid() ? tabIndex->m_cardAbsoluteRectCache : D2D1_RECT_F{};
    }

    D2D1_RECT_F TabGroup::cardCaptionAbsoluteRect(TabIndexParam tabIndex) const
    {
        auto state = getCardState(tabIndex);
        float roundRaidus = appearance().tabBar.card.main[(size_t)state].geometry.roundRadius;

        if (state == CardState::Active) // Clip margins.
        {
            return math_utils::stretch(cardAbsoluteRect(tabIndex), { -roundRaidus, 0.0f });
        }
        else // Dormant, Hover: Crop only the upper half.
        {
            return math_utils::increaseBottom(cardAbsoluteRect(tabIndex), -roundRaidus);
        }
    }

    D2D1_RECT_F TabGroup::separatorAbsoluteRect(TabIndexParam tabIndex) const
    {
        auto& setting = appearance().tabBar.separator.geometry;

        return math_utils::rect(math_utils::offset(
            math_utils::rightTop(cardAbsoluteRect(tabIndex)), setting.offset), setting.size);
    }

    D2D1_RECT_F TabGroup::moreCardsButtonAbsoluteRect() const
    {
        auto& setting = appearance().tabBar.moreCards.control.button.geometry;

        return math_utils::rect(math_utils::offset(
            math_utils::rightTop(cardBarAbsoluteRect()), setting.offset), setting.size);
    }

    SharedPtr<Window> TabGroup::promoteTabToWindow(size_t index)
    {
        if (index >= 0 && index < m_tabs.size())
        {
            return promoteTabToWindow({ &m_tabs, index });
        }
        else return nullptr;
    }

    SharedPtr<Window> TabGroup::promoteTabToWindow(TabIndexParam tabIndex)
    {
        auto rect = math_utils::increaseTop
        (
            tabIndex->content->absoluteRect(), -Window::nonClientAreaDefaultHeight()
        );
        auto w = makeUIObject<Window>(tabIndex->caption->title(), rect);

        tabIndex->content->setPrivateEnabled(true);

        w->setContent(tabIndex->content);

        removeTab(tabIndex, 1);

        w->minimalWidthHint = std::max(minimalWidth(), Window::nonClientAreaMinimalWidth());
        w->minimalHeightHint = minimalHeight() + Window::nonClientAreaDefaultHeight();

        w->maximalWidthHint = std::max(maximalWidth(), Window::nonClientAreaMinimalWidth());
        w->maximalHeightHint = maximalHeight() + Window::nonClientAreaDefaultHeight();

        return w;
    }

    void TabGroup::triggerTabPromoting(MouseMoveEvent& e)
    {
        THROW_IF_NULL(Application::g_app);

        if (m_draggedCardTabIndex.valid() && m_draggedCardTabIndex->caption->promotable)
        {
            auto w = promoteTabToWindow(m_draggedCardTabIndex);

            // Ensure the cursor is within the caption of the promoted window,
            // so we can send a pseudo mouse-button event to trigger dragging.

            D2D1_POINT_2F offset =
            {
                -Window::nonClientAreaMinimalWidth() * 0.5f,
                -w->nonClientAreaHeight() * 0.5f
            };
            w->setPosition(math_utils::offset(e.cursorPoint, offset));

            auto focus = Application::FocusType::Mouse;
            Application::g_app->focusUIObject(focus, w);

            MouseButtonEvent immediateMouseButton = {};
            immediateMouseButton.cursorPoint = e.cursorPoint;
            immediateMouseButton.state = { MouseButtonEvent::State::Flag::LeftDown };

            w->onMouseButton(immediateMouseButton);

            w->registerTabGroup(std::dynamic_pointer_cast<TabGroup>(shared_from_this()));

            if (f_onTriggerTabPromoting) f_onTriggerTabPromoting(this, w.get());
        }
    }

    bool TabGroup::isAssociatedWindowDraggedAbove() const
    {
        if (!associatedWindow.expired())
        {
            auto targetWindow = associatedWindow.lock();
            auto targetTabGroup = targetWindow->associatedTabGroup.lock();

            return cpp_lang_utils::isMostDerivedEqual(targetTabGroup, shared_from_this());
        }
        else return false;
    }

    void TabGroup::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        if (m_activeCardTabIndex.valid())
        {
            if (m_activeCardTabIndex->content->isD2d1ObjectVisible())
            {
                m_activeCardTabIndex->content->onRendererDrawD2d1Layer(rndr);
            }
            if (m_activeCardTabIndex.index < m_candidateTabCount)
            {
                //////////////////////
                // Active-Card Mask //
                //////////////////////

                activeCard.mask.beginDraw(rndr->d2d1DeviceContext());
                {
                    auto& setting = appearance().tabBar.card.main[(size_t)CardState::Active];

                    resource_utils::solidColorBrush()->SetColor(setting.background.color);
                    resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

                    rndr->d2d1DeviceContext()->FillGeometry
                    (
                    /* geometry */ activeCard.pathGeo.Get(),
                    /* brush    */ resource_utils::solidColorBrush()
                    );
                }
                activeCard.mask.endDraw(rndr->d2d1DeviceContext());

                /////////////////////
                // More-Cards Mask //
                /////////////////////

                moreCards.mask.beginDraw(rndr->d2d1DeviceContext());
                {
                    auto state = getMoreCardsButtonState();
                    auto& setting = appearance().tabBar.moreCards.control;

                    //-------------------------------------------------------------------------
                    // Button
                    //-------------------------------------------------------------------------

                    auto& buttonBackground = setting.button.background[(size_t)state];

                    resource_utils::solidColorBrush()->SetColor(buttonBackground.color);
                    resource_utils::solidColorBrush()->SetOpacity(buttonBackground.opacity);

                    D2D1_ROUNDED_RECT roundedRect =
                    {
                        math_utils::sizeOnlyRect(setting.button.geometry.size),
                        setting.button.geometry.roundRadius,
                        setting.button.geometry.roundRadius
                    };
                    rndr->d2d1DeviceContext()->FillRoundedRectangle
                    (
                    /* roundedRect */ roundedRect,
                    /* brush       */ resource_utils::solidColorBrush()
                    );
                    //-------------------------------------------------------------------------
                    // Icon
                    //-------------------------------------------------------------------------

                    auto& iconBackground = setting.icon.background[(size_t)state];

                    resource_utils::solidColorBrush()->SetColor(iconBackground.color);
                    resource_utils::solidColorBrush()->SetOpacity(iconBackground.opacity);

                    auto& topRect = setting.icon.geometry.topRect;
                    rndr->d2d1DeviceContext()->FillRectangle
                    (
                    /* rect  */ math_utils::rect(topRect.offset, topRect.size),
                    /* brush */ resource_utils::solidColorBrush()
                    );
                    rndr->d2d1DeviceContext()->FillGeometry
                    (
                    /* geometry */ moreCards.pathGeo.Get(),
                    /* brush    */ resource_utils::solidColorBrush()
                    );
                }
                moreCards.mask.endDraw(rndr->d2d1DeviceContext());
            }
        }
    }

    void TabGroup::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////////////
        // Card-Bar Panel //
        ////////////////////
        {
            auto& setting = appearance().tabBar.card.main[(size_t)CardState::Dormant];

            resource_utils::solidColorBrush()->SetColor(setting.background.color);
            resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

            D2D1_ROUNDED_RECT roundedRect =
            {
                cardBarAbsoluteRect(),
                setting.geometry.roundRadius,
                setting.geometry.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ roundedRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        ////////////////
        // Hover-Card //
        ////////////////

        if (m_hoverCardTabIndex.valid())
        {
            if (m_hoverCardTabIndex != m_activeCardTabIndex)
            {
                auto& setting = appearance().tabBar.card.main[(size_t)CardState::Hover];

                resource_utils::solidColorBrush()->SetColor(setting.background.color);
                resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

                D2D1_ROUNDED_RECT roundedRect =
                {
                    cardAbsoluteRect(m_hoverCardTabIndex),
                    setting.geometry.roundRadius,
                    setting.geometry.roundRadius
                };
                rndr->d2d1DeviceContext()->FillRoundedRectangle
                (
                /* roundedRect */ roundedRect,
                /* brush       */ resource_utils::solidColorBrush()
                );
            }
        }
        /////////////////
        // Active-Card //
        /////////////////

        if (m_activeCardTabIndex.valid() && m_activeCardTabIndex.index < m_candidateTabCount)
        {
            //-------------------------------------------------------------------------
            // Shadow
            //-------------------------------------------------------------------------

            activeCard.mask.color = appearance().tabBar.card.activeShadowColor;

            activeCard.mask.configEffectInput(resource_utils::shadowEffect());

            auto shadowPosition = math_utils::leftTop(cardAbsoluteRect(m_activeCardTabIndex));

            rndr->d2d1DeviceContext()->DrawImage
            (
            /* effect       */ resource_utils::shadowEffect(),
            /* targetOffset */ shadowPosition
            );
            //-------------------------------------------------------------------------
            // Entity
            //-------------------------------------------------------------------------

            auto& setting = appearance().tabBar.card.main[(size_t)CardState::Active];

            rndr->d2d1DeviceContext()->DrawBitmap
            (
            /* bitmap               */ activeCard.mask.data.Get(),
            /* destinationRectangle */ cardAbsoluteRect(m_activeCardTabIndex),
            /* opacity              */ setting.background.opacity,
            /* interpolationMode    */ activeCard.mask.getInterpolationMode()
            );
        }
        ////////////////
        // Background //
        ////////////////
        {
            auto& background = appearance().background;

            resource_utils::solidColorBrush()->SetColor(background.color);
            resource_utils::solidColorBrush()->SetOpacity(background.opacity);

            ResizablePanel::drawBackground(rndr);
        }
        /////////////
        // Content //
        /////////////

        if (m_activeCardTabIndex.valid())
        {
            if (m_activeCardTabIndex->content->isD2d1ObjectVisible())
            {
                m_activeCardTabIndex->content->onRendererDrawD2d1Object(rndr);
            }
        }
        ///////////////////
        // Miscellaneous //
        ///////////////////
        {
            for (TabIndex tabIndex = { &m_tabs, 0 }; tabIndex < m_candidateTabCount; ++tabIndex)
            {
                auto currState = getCardState(tabIndex);
                auto nextState = getCardState(tabIndex.getNext());

                //-------------------------------------------------------------------------
                // Caption
                //-------------------------------------------------------------------------
                if (tabIndex->caption->isD2d1ObjectVisible())
                {
                    tabIndex->caption->onRendererDrawD2d1Object(rndr);
                }
                //-------------------------------------------------------------------------
                // Separators
                //-------------------------------------------------------------------------
                if (currState == CardState::Dormant && nextState == CardState::Dormant)
                {
                    if (tabIndex != m_candidateTabCount - 1)
                    {
                        auto& setting = appearance().tabBar.separator;

                        resource_utils::solidColorBrush()->SetColor(setting.background.color);
                        resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

                        rndr->d2d1DeviceContext()->FillRectangle
                        (
                        /* rect  */ separatorAbsoluteRect(tabIndex),
                        /* brush */ resource_utils::solidColorBrush()
                        );
                    }
                }
            }
            //-------------------------------------------------------------------------
            // More-Cards
            //-------------------------------------------------------------------------

            rndr->d2d1DeviceContext()->DrawBitmap
            (
            /* bitmap               */ moreCards.mask.data.Get(),
            /* destinationRectangle */ moreCardsButtonAbsoluteRect(),
            /* opacity              */ moreCards.mask.opacity,
            /* interpolationMode    */ moreCards.mask.getInterpolationMode()
            );
        }
        ////////////////////////
        // Mask below Window //
        ///////////////////////

        if (isAssociatedWindowDraggedAbove())
        {
            auto& maskSetting = appearance().maskWhenBelowDragWindow;

            resource_utils::solidColorBrush()->SetColor(maskSetting.color);
            resource_utils::solidColorBrush()->SetOpacity(maskSetting.opacity);

            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ { m_absoluteRect, roundRadiusX, roundRadiusY },
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
    }

    void TabGroup::drawD2d1ObjectPosterior(Renderer* rndr)
    {
        /////////////
        // Outline //
        /////////////

        resource_utils::solidColorBrush()->SetColor(appearance().stroke.color);
        resource_utils::solidColorBrush()->SetOpacity(appearance().stroke.opacity);

        // In general, the round-radius of the tab-group is 0, so we can draw
        // left, right and bottom lines separately to hide the topmost border.

        auto leftTop = absolutePosition();

        float selfWidth = width();
        float selfHeight = height();

        auto& stroke = appearance().stroke;

        auto point00 = math_utils::offset(leftTop, { stroke.width * 0.5f, 0.0f });
        auto point01 = math_utils::offset(point00, { 0.0f, selfHeight });

        auto point10 = math_utils::offset(leftTop, { selfWidth - stroke.width * 0.5f, 0.0f });
        auto point11 = math_utils::offset(point10, { 0.0f, selfHeight });

        auto point20 = math_utils::offset(leftTop, { 0.0f, selfHeight - stroke.width * 0.5f });
        auto point21 = math_utils::offset(point20, { selfWidth, 0.0f });

        rndr->d2d1DeviceContext()->DrawLine
        (
        /* point0      */ point00,
        /* point1      */ point01,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ stroke.width
        );
        rndr->d2d1DeviceContext()->DrawLine
        (
        /* point0      */ point10,
        /* point1      */ point11,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ stroke.width
        );
        rndr->d2d1DeviceContext()->DrawLine
        (
        /* point0      */ point20,
        /* point1      */ point21,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ stroke.width
        );
        ///////////////////////
        // Content Posterior //
        //////////////////////

        if (m_activeCardTabIndex.valid())
        {
            if (m_activeCardTabIndex->content->isD2d1ObjectVisible())
            {
                m_activeCardTabIndex->content->drawD2d1ObjectPosterior(rndr);
            }
        }
        ResizablePanel::drawD2d1ObjectPosterior(rndr);
    }

    bool TabGroup::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isOverlapped(p, sizingFrameExtendedRect(cardBarExtendedAbsoluteRect()));
    }

    void TabGroup::onSizeHelper(SizeEvent& e)
    {
        ResizablePanel::onSizeHelper(e);

        if (m_activeCardTabIndex.valid())
        {
            m_activeCardTabIndex->content->setSize(size());
        }
        updateCandidateTabInfo();
    }

    void TabGroup::onMoveHelper(MoveEvent& e)
    {
        ResizablePanel::onMoveHelper(e);

        updateCandidateTabInfo();
    }

    void TabGroup::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ResizablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);

        for (auto& tab : m_tabs)
        {
            if (tab.m_previewItem->parent().expired())
            {
                tab.m_previewItem->onChangeThemeStyle(style);
            }
        } // The managed preview-items will be updated by the preview-panel.
    }

    void TabGroup::onMouseMoveHelper(MouseMoveEvent& e)
    {
        ResizablePanel::onMouseMoveHelper(e);

        auto& p = e.cursorPoint;

        if (math_utils::isOverlapped(p, cardBarExtendedCardBarAbsoluteRect()))
        {
            if (m_draggedCardTabIndex.valid())
            {
                if (e.buttonState.leftPressed)
                {
                    auto nextCardTabIndex = selfcoordOffsetToCardTabIndex(absoluteToSelfCoord(p).x);

                    if (nextCardTabIndex.valid() && nextCardTabIndex != m_draggedCardTabIndex)
                    {
                        swapTab(m_draggedCardTabIndex, nextCardTabIndex);

                        selectTab(nextCardTabIndex); // m_activeCardTabIndex is updated in this.

                        m_hoverCardTabIndex = m_draggedCardTabIndex = nextCardTabIndex;
                    }
                }
            }
            else // No card being dragged.
            {
                m_hoverCardTabIndex.invalidate();

                for (TabIndex tabIndex = { &m_tabs, 0 }; tabIndex < m_candidateTabCount; ++tabIndex)
                {
                    if (math_utils::isInside(p, cardAbsoluteRect(tabIndex)))
                    {
                        m_hoverCardTabIndex = tabIndex;
                    }
                }
                if (!math_utils::isInside(p, moreCardsButtonAbsoluteRect()))
                {
                    m_isMoreCardsButtonHover = false;
                    m_isMoreCardsButtonDown = false;
                }
                else m_isMoreCardsButtonHover = true;
            }
        }
        else // Not above the card-bar.
        {
            m_hoverCardTabIndex.invalidate();

            if (e.buttonState.leftPressed)
            {
                triggerTabPromoting(e);
            }
            m_draggedCardTabIndex.invalidate();

            m_isMoreCardsButtonHover = false;
            m_isMoreCardsButtonDown = false;
        }
    }

    void TabGroup::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ResizablePanel::onMouseLeaveHelper(e);

        m_hoverCardTabIndex.invalidate();

        if (e.buttonState.leftPressed)
        {
            triggerTabPromoting(e);
        }
        m_draggedCardTabIndex.invalidate();

        m_isMoreCardsButtonHover = false;
        m_isMoreCardsButtonDown = false;
    }

    void TabGroup::onMouseButtonHelper(MouseButtonEvent& e)
    {
        ResizablePanel::onMouseButtonHelper(e);

        auto& p = e.cursorPoint;

        if (e.state.leftDown() || e.state.leftDblclk())
        {
            if (m_hoverCardTabIndex.valid())
            {
                if (m_hoverCardTabIndex->caption->draggable &&
                   (!m_hoverCardTabIndex->caption->closable ||
                    !m_hoverCardTabIndex->caption->m_isCloseButtonHover))
                {
                    m_draggedCardTabIndex = m_hoverCardTabIndex;
                }
            }
            m_isMoreCardsButtonDown = m_isMoreCardsButtonHover;
        }
        else if (e.state.leftUp())
        {
            if (m_isMoreCardsButtonDown)
            {
                m_isMoreCardsButtonDown = false;

                updatePreviewPanelItems();
                m_previewPanel->setActivated(true);
            }
            m_draggedCardTabIndex.invalidate();
        }
    }
}
