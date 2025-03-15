#include "Common/Precompile.h"

#include "UIKit/Window.h"

#include "Common/CppLangUtils/PointerEquality.h"
#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"

#include "UIKit/Application.h"
#include "UIKit/IconLabel.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/TabCaption.h"
#include "UIKit/TabGroup.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    Window::Window(
        ShrdPtrParam<IconLabel> caption,
        const D2D1_RECT_F& rect,
        float captionPanelHeight,
        float decorativeBarHeight)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        DraggablePanel(rect, resource_utils::solidColorBrush()),
        ResizablePanel(rect, resource_utils::solidColorBrush()),
        m_caption(caption),
        m_captionPanelHeight(captionPanelHeight),
        m_decorativeBarHeight(decorativeBarHeight)
    {
        m_takeOverChildrenDrawing = true;

        transform(math_utils::adaptMaxSize(rect, minimalSize()));

        ///////////////////////////
        // Load Cached Resources //
        ///////////////////////////

        drawBufferRes.loadShadowMask();
        drawBufferRes.loadBrush();

        decorativeBarRes.loadBrush();
    }

    Window::Window(
        WstrParam title,
        const D2D1_RECT_F& rect,
        float captionPanelHeight,
        float decorativeBarHeight)
        :
        Window(
            IconLabel::labelExpandedLayout(title),
            rect,
            captionPanelHeight,
            decorativeBarHeight) { }

    void Window::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        ///////////////////////////
        // Init Children Objects //
        ///////////////////////////

        if (!m_caption)
        {
            m_caption = IconLabel::labelExpandedLayout(L"Untitled");
        }
        addUIObject(m_caption);

        m_caption->transform(captionTitleSelfcoordRect());
    }

    void Window::DrawBufferRes::loadShadowMask()
    {
        Window* w = m_master;
        THROW_IF_NULL(w);

        shadowMask.loadBitmap(w->size());
    }

    MaskObject& Window::DrawBufferRes::mask()
    {
        return shadowMask;
    }

    ShadowMask& Window::DrawBufferRes::shadow()
    {
        return shadowMask;
    }

    void Window::DrawBufferRes::loadBrush()
    {
        Window* w = m_master;
        THROW_IF_NULL(w);

        auto rndr = Application::g_app->dx12Renderer();
        auto context = rndr->d2d1DeviceContext();

        THROW_IF_FAILED(context->CreateBitmapBrush
        (
        /* bitmap      */ mask().data.Get(),
        /* bitmapBrush */ &brush
        ));
    }

    void Window::DecorativeBarRes::loadBrush()
    {
        Window* w = m_master;
        THROW_IF_NULL(w);

        THROW_IF_NULL(Application::g_app);

        auto& colors = w->appearance().decorativeBar.gradientColors;
        D2D1_GRADIENT_STOP stop[] =
        {
            { 0.0f, colors[0] },
            { 0.5f, colors[1] },
            { 1.0f, colors[2] },
        };
        auto rndr = Application::g_app->dx12Renderer();
        auto context = rndr->d2d1DeviceContext();

        ComPtr<ID2D1GradientStopCollection> collection = {};
        THROW_IF_FAILED(context->CreateGradientStopCollection
        (
        /* gradientStops          */ stop,
        /* gradientStopsCount     */ _countof(stop),
        /* gradientStopCollection */ &collection
        ));
        THROW_IF_FAILED(context->CreateLinearGradientBrush
        (
        /* linearGradientBrushProperties */ {},
        /* gradientStopCollection        */ collection.Get(),
        /* linearGradientBrush           */ &brush)
        );
    }

    void Window::onMinimize()
    {
        onMinimizeHelper();

        if (f_onMinimize) f_onMinimize(this);
    }

    void Window::onMaximize()
    {
        onMaximizeHelper();

        if (f_onMaximize) f_onMaximize(this);
    }

    void Window::onRestore()
    {
        onRestoreHelper();

        if (f_onRestore) f_onRestore(this);
    }

    void Window::onClose()
    {
        onCloseHelper();

        if (f_onClose) f_onClose(this);
    }

    void Window::onMinimizeHelper()
    {
        // This method intentionally left blank.
    }

    void Window::onMaximizeHelper()
    {
        // This method intentionally left blank.
    }

    void Window::onRestoreHelper()
    {
        // This method intentionally left blank.
    }

    void Window::onCloseHelper()
    {
        // This method intentionally left blank.
    }

    const SharedPtr<IconLabel>& Window::caption() const
    {
        return m_caption;
    }

    void Window::setCaption(ShrdPtrParam<IconLabel> caption)
    {
        if (caption && !cpp_lang_utils::isMostDerivedEqual(caption, m_caption))
        {
            removeUIObject(m_caption);

            m_caption = caption;
            addUIObject(m_caption);

            m_caption->transform(captionTitleSelfcoordRect());
        }
    }

    void Window::setContent(ShrdPtrParam<Panel> uiobj)
    {
        if (!cpp_lang_utils::isMostDerivedEqual(uiobj, m_content))
        {
            removeUIObject(m_content);

            m_content = uiobj;
            addUIObject(m_content);

            if (m_content) m_content->transform(clientAreaSelfcoordRect());
        }
    }

    D2D1_RECT_F Window::captionPanelAbsoluteRect() const
    {
        return
        {
            m_absoluteRect.left,
            m_absoluteRect.top,
            m_absoluteRect.right,
            m_absoluteRect.top + m_captionPanelHeight
        };
    }

    D2D1_RECT_F Window::decorativeBarAbsoluteRect() const
    {
        return
        {
            m_absoluteRect.left,
            m_absoluteRect.top + m_captionPanelHeight,
            m_absoluteRect.right,
            m_absoluteRect.top + nonClientAreaHeight()
        };
    }

    D2D1_RECT_F Window::captionTitleSelfcoordRect() const
    {
        return
        {
            buttonPanelLeftmostOffset(),
            0.0f,
            std::max(width() - buttonPanelLeftmostOffset(), buttonPanelLeftmostOffset()),
            m_captionPanelHeight
        };
    }

    float Window::captionPanelHeight() const
    {
        return m_captionPanelHeight;
    }

    void Window::setCaptionPanelHeight(float value)
    {
        m_captionPanelHeight = value;

        m_caption->transform(captionTitleSelfcoordRect());
        if (m_content) m_content->transform(clientAreaSelfcoordRect());
    }

    float Window::decorativeBarHeight() const
    {
        return m_decorativeBarHeight;
    }

    void Window::setDecorativeBarHeight(float value)
    {
        m_decorativeBarHeight = value;

        m_caption->transform(captionTitleSelfcoordRect());
        if (m_content) m_content->transform(clientAreaSelfcoordRect());
    }

    float Window::clientAreaHeight() const
    {
        return height() - nonClientAreaHeight();
    }

    D2D1_RECT_F Window::clientAreaSelfcoordRect() const
    {
        return { 0.0f, nonClientAreaHeight(), width(), height() };
    }

    float Window::nonClientAreaHeight() const
    {
        return m_captionPanelHeight + m_decorativeBarHeight;
    }

    D2D1_RECT_F Window::nonClientAreaSelfcoordRect() const
    {
        return { 0.0f, 0.0f, width(), nonClientAreaHeight() };
    }

    D2D1_RECT_F Window::nonClientAreaMinimalSelfcoordRect() const
    {
        return { 0.0f, 0.0f, nonClientAreaMinimalWidth(), nonClientAreaHeight() };
    }

    D2D1_RECT_F Window::button1AbsoluteRect() const
    {
        return
        {
            m_absoluteRect.right - buttonPanelLeftmostOffset(),
            m_absoluteRect.top,
            m_absoluteRect.right - buttonPanelLeftmostOffset() + button1Width(),
            m_absoluteRect.top + buttonHeight()
        };
    }

    D2D1_RECT_F Window::button2AbsoluteRect() const
    {
        return
        {
            m_absoluteRect.right - buttonPanelLeftmostOffset() + button1Width(),
            m_absoluteRect.top,
            m_absoluteRect.right - buttonPanelRightmostOffset() - button3Width(),
            m_absoluteRect.top + buttonHeight()
        };
    }

    D2D1_RECT_F Window::button3AbsoluteRect() const
    {
        return
        {
            m_absoluteRect.right - buttonPanelRightmostOffset() - button3Width(),
            m_absoluteRect.top,
            m_absoluteRect.right - buttonPanelRightmostOffset(),
            m_absoluteRect.top + buttonHeight()
        };
    }

    D2D1_RECT_F Window::minimizeIconAbsoluteRect() const
    {
        return math_utils::moveVertex(minimizeButtonAbsoluteRect(), minimizeIconVertexOffset());
    }

    D2D1_RECT_F Window::minimizeButtonAbsoluteRect() const
    {
        return button1AbsoluteRect();
    }

    D2D1_RECT_F Window::maximizeIconAbsoluteRect() const
    {
        return math_utils::moveVertex(maximizeButtonAbsoluteRect(), maximizeIconVertexOffset());
    }

    D2D1_RECT_F Window::maximizeButtonAbsoluteRect() const
    {
        return button2AbsoluteRect();
    }

    D2D1_RECT_F Window::restoreIconAbsoluteRect() const
    {
        return math_utils::moveVertex(maximizeButtonAbsoluteRect(), restoreIconVertexOffset());
    }

    D2D1_RECT_F Window::restoreButtonAbsoluteRect() const
    {
        return button2AbsoluteRect();
    }

    D2D1_RECT_F Window::closeIconAbsoluteRect() const
    {
        return math_utils::moveVertex(closeButtonAbsoluteRect(), closeIconVertexOffset());
    }

    D2D1_RECT_F Window::closeButtonAbsoluteRect() const
    {
        return button3AbsoluteRect();
    }

    Window::DisplayState Window::displayState() const
    {
        return m_displayState;
    }

    void Window::setDisplayState(DisplayState state)
    {
        if (state != m_displayState)
        {
            switch (state = m_displayState)
            {
            case Normal: onRestore(); break;
            case Minimized: onMinimize(); break;
            case Maximized: onMaximize(); break;
            default: break;
            }
        }
    }

    bool Window::isPerformSpecialOperation() const
    {
        return m_isDragging || isSizing();
    }

    Window::ButtonState Window::getButton1State(bool isHover, bool isDown) const
    {
        if (isDown) return ButtonState::Down;
        else if (isHover) return ButtonState::Hover;
        else return ButtonState::Idle;
    }

    Window::ButtonState Window::getButton2State(bool isHover, bool isDown) const
    {
        if (isDown) return ButtonState::Down;
        else if (isHover) return ButtonState::Hover;
        else return ButtonState::Idle;
    }

    Window::ButtonState Window::getButton3State(bool isHover, bool isDown) const
    {
        if (isDown) return ButtonState::CloseDown;
        else if (isHover) return ButtonState::CloseHover;
        else return ButtonState::CloseIdle;
    }

    void Window::setIconBrushState(ButtonState state)
    {
        auto& foreground = appearance().buttonPanel[(size_t)state].foreground;

        resource_utils::solidColorBrush()->SetColor(foreground.color);
        resource_utils::solidColorBrush()->SetOpacity(foreground.opacity);
    }

    void Window::setButtonBrushState(ButtonState state)
    {
        auto& background = appearance().buttonPanel[(size_t)state].background;

        resource_utils::solidColorBrush()->SetColor(background.color);
        resource_utils::solidColorBrush()->SetOpacity(background.opacity);
    }

    void Window::registerTabGroup(WeakPtrParam<TabGroup> tg)
    {
        m_registeredTabGroups.insert(tg);
    }

    void Window::unregisterTabGroup(WeakPtrParam<TabGroup> tg)
    {
        m_registeredTabGroups.erase(tg);
    }

    void Window::handleMouseMoveForRegisteredTabGroups(MouseMoveEvent& e)
    {
        associatedTabGroup.reset();
        if (!m_isDragging) return;

        for (auto tgItor = m_registeredTabGroups.begin(); tgItor != m_registeredTabGroups.end(); )
        {
            if (!tgItor->expired())
            {
                auto tabGroup = tgItor->lock();
                if (math_utils::isInside(e.cursorPoint, tabGroup->cardBarExtendedCardBarAbsoluteRect()))
                {
                    associatedTabGroup = tabGroup;
                    tabGroup->associatedWindow = std::dynamic_pointer_cast<Window>(shared_from_this());
                }
                else if (cpp_lang_utils::isMostDerivedEqual(tabGroup->associatedWindow.lock(), shared_from_this()))
                {
                    tabGroup->associatedWindow.reset();
                }
                tgItor++;
            }
            else tgItor = m_registeredTabGroups.erase(tgItor);
        }
    }

    void Window::handleMouseButtonForRegisteredTabGroups(MouseButtonEvent& e)
    {
        if (e.state.leftUp())
        {
            if (m_content && !associatedTabGroup.expired())
            {
                auto tabGroup = associatedTabGroup.lock();

                // Retain this to avoid abrupt destruction after release(),
                // otherwise the following demoting operation may collapse.
                auto temporaryLocked = shared_from_this();

                if (release())
                {
                    auto caption = makeUIObject<TabCaption>(m_caption);
                    caption->promotable = true;

                    tabGroup->insertTab({ caption, m_content });
                    tabGroup->selectTab(0);

                    if (f_onTriggerTabDemoting) f_onTriggerTabDemoting(this, tabGroup.get());
                }
            }
        }
    }

    float Window::minimalWidth() const
    {
        if (minimalWidthHint.has_value())
        {
            auto userWidth = minimalWidthHint.value();
            if (userWidth > nonClientAreaMinimalWidth())
            {
                return userWidth;
            }
        }
        return nonClientAreaMinimalWidth();
    }

    float Window::minimalHeight() const
    {
        if (minimalHeightHint.has_value())
        {
            auto userHeight = minimalHeightHint.value();
            if (userHeight > nonClientAreaHeight())
            {
                return userHeight;
            }
        }
        return nonClientAreaHeight();
    }

    void Window::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        Panel::drawChildrenLayers(rndr);

        auto& mask = drawBufferRes.mask();

        auto maskDrawTrans = D2D1::Matrix3x2F::Translation
        (
            -m_absoluteRect.left, -m_absoluteRect.top
        );
        mask.beginDraw(rndr->d2d1DeviceContext(), maskDrawTrans);
        {
            ////////////////
            // Background //
            ////////////////
            {
                auto& background = appearance().background;

                resource_utils::solidColorBrush()->SetColor(background.color);
                resource_utils::solidColorBrush()->SetOpacity(background.opacity);

                ResizablePanel::drawBackground(rndr);
            }
            /////////////////////
            // Non-client Area //
            /////////////////////
            {
                //------------------------------------------------------------------
                // Caption Panel
                //------------------------------------------------------------------
                {
                    auto& background = appearance().captionPanel.background;

                    resource_utils::solidColorBrush()->SetColor(background.color);
                    resource_utils::solidColorBrush()->SetOpacity(background.opacity);

                    rndr->d2d1DeviceContext()->FillRectangle
                    (
                    /* rect  */ captionPanelAbsoluteRect(),
                    /* brush */ resource_utils::solidColorBrush()
                    );
                }
                //------------------------------------------------------------------
                // The caption title will be drawn as a child.
                //------------------------------------------------------------------

                //------------------------------------------------------------------
                // Decorative Bar
                //------------------------------------------------------------------
                {
                    auto& brush = decorativeBarRes.brush;

                    auto rect = decorativeBarAbsoluteRect();

                    brush->SetStartPoint({ rect.left, rect.top });
                    brush->SetEndPoint({ rect.right, rect.top });

                    rndr->d2d1DeviceContext()->FillRectangle
                    (
                    /* rect  */ rect,
                    /* brush */ brush.Get()
                    );
                }
            }
            ////////////////
            // 3 Brothers //
            ////////////////
            {
                //------------------------------------------------------------------
                // Minimize Button
                //------------------------------------------------------------------

                if (button1Enabled)
                {
                    auto state = getButton1State(m_isButton1Hover, m_isButton1Down);

                    setButtonBrushState(state);

                    // Background
                    rndr->d2d1DeviceContext()->FillRectangle
                    (
                    /* rect  */ minimizeButtonAbsoluteRect(),
                    /* brush */ resource_utils::solidColorBrush()
                    );
                    setIconBrushState(state);

                    // Center Dash
                    rndr->d2d1DeviceContext()->FillRectangle
                    (
                    /* rect  */ minimizeIconAbsoluteRect(),
                    /* brush */ resource_utils::solidColorBrush()
                    );
                }
                //------------------------------------------------------------------
                // Maximize/Restore Button
                //------------------------------------------------------------------

                if (button2Enabled)
                {
                    auto state = getButton1State(m_isButton2Hover, m_isButton2Down);

                    setButtonBrushState(state);

                    // Background
                    rndr->d2d1DeviceContext()->FillRectangle
                    (
                    /* rect  */ maximizeButtonAbsoluteRect(),
                    /* brush */ resource_utils::solidColorBrush()
                    );
                    setIconBrushState(state);

                    // Maximize Button
                    if (m_displayState == Normal)
                    {
                        rndr->d2d1DeviceContext()->DrawRectangle
                        (
                        /* rect        */ maximizeIconAbsoluteRect(),
                        /* brush       */ resource_utils::solidColorBrush(),
                        /* strokeWidth */ maximizeIconStrokeWidth()
                        );
                    }
                    else // Restore Button
                    {
                        auto rect = restoreIconAbsoluteRect();

                        // Square
                        rndr->d2d1DeviceContext()->DrawRectangle
                        (
                        /* rect        */ rect,
                        /* brush       */ resource_utils::solidColorBrush(),
                        /* strokeWidth */ restoreIconStrokeWidth()
                        );
                        D2D1_POINT_2F point00 =
                        {
                            rect.left,
                            rect.top - restoreOrnamentOffset()
                        };
                        D2D1_POINT_2F point10 =
                        {
                            rect.right + restoreOrnamentOffset(),
                            rect.bottom
                        };
                        D2D1_POINT_2F crossPoint = { point10.x, point00.y };

                        auto point01 = math_utils::increaseX(
                            crossPoint, -restoreIconStrokeWidth() * 0.5f);

                        // Top Dash
                        rndr->d2d1DeviceContext()->DrawLine
                        (
                        /* point0      */ point00,
                        /* point1      */ point01,
                        /* brush       */ resource_utils::solidColorBrush(),
                        /* strokeWidth */ restoreIconStrokeWidth()
                        );

                        auto point11 = math_utils::increaseY(
                            crossPoint, -restoreIconStrokeWidth() * 0.5f);

                        // Right Dash
                        rndr->d2d1DeviceContext()->DrawLine
                        (
                        /* point0      */ point10,
                        /* point1      */ point11,
                        /* brush       */ resource_utils::solidColorBrush(),
                        /* strokeWidth */ restoreIconStrokeWidth()
                        );
                    }
                }
                //------------------------------------------------------------------
                // Close Button
                //------------------------------------------------------------------
                
                if (button3Enabled)
                {
                    auto state = getButton3State(m_isButton3Hover, m_isButton3Down);

                    setButtonBrushState(state);

                    // Background
                    {
                        rndr->d2d1DeviceContext()->FillRectangle
                        (
                        /* rect  */ closeButtonAbsoluteRect(),
                        /* brush */ resource_utils::solidColorBrush()
                        );
                        setIconBrushState(state);
                    }
                    // Foreground
                    {
                        auto rect = closeIconAbsoluteRect();

                        // Main Diagonal
                        rndr->d2d1DeviceContext()->DrawLine
                        (
                        /* point0      */ { rect.left, rect.top },
                        /* point1      */ { rect.right, rect.bottom },
                        /* brush       */ resource_utils::solidColorBrush(),
                        /* strokeWidth */ closeIconStrokeWidth()
                        );
                        // Back Diagonal
                        rndr->d2d1DeviceContext()->DrawLine
                        (
                        /* point0      */ { rect.right, rect.top },
                        /* point1      */ { rect.left, rect.bottom },
                        /* brush       */ resource_utils::solidColorBrush(),
                        /* strokeWidth */ closeIconStrokeWidth()
                        );
                    }
                }
            }
            //////////////
            // Children //
            //////////////
            {
                Panel::drawChildrenObjects(rndr);
            }
        }
        mask.endDraw(rndr->d2d1DeviceContext());
    }

    void Window::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////
        // Shadow //
        ////////////
        {
            auto& shadow = drawBufferRes.shadow();

            if (shadow.enabled && associatedTabGroup.expired())
            {
                auto& setting = appearance().shadow;

                shadow.color = setting.color;
                shadow.standardDeviation = setting.standardDeviation;

                shadow.configEffectInput(resource_utils::shadowEffect());

                auto offset = math_utils::offset(absolutePosition(), shadow.offset);

                rndr->d2d1DeviceContext()->DrawImage
                (
                /* effect       */ resource_utils::shadowEffect(),
                /* targetOffset */ offset
                );
            }
        }
        /////////////
        // Content //
        /////////////
        {
            auto& mask = drawBufferRes.mask();
            auto& brush = drawBufferRes.brush;

            float maskOpacity = {};
            if (associatedTabGroup.expired())
            {
                maskOpacity = mask.opacity;
            }
            else maskOpacity = appearance().maskOpacityAboveTabGroup;

            brush->SetOpacity(maskOpacity);

            auto mode = mask.getInterpolationMode();
            brush->SetInterpolationMode1(mode);

            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ { m_absoluteRect, roundRadiusX, roundRadiusY },
            /* brush       */ brush.Get()
            );
        }
    }

    void Window::drawD2d1ObjectPosterior(Renderer* rndr)
    {
        /////////////
        // Outline //
        /////////////
        {
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
        ResizablePanel::drawD2d1ObjectPosterior(rndr);
    }

    bool Window::releaseUIObjectHelper(ShrdPtrParam<Panel> uiobj)
    {
        if (cpp_lang_utils::isMostDerivedEqual(uiobj, m_caption)) return false;

        return Panel::releaseUIObjectHelper(uiobj);
    }

    void Window::onSizeHelper(SizeEvent& e)
    {
        ResizablePanel::onSizeHelper(e);

        /////////////////////////////
        // Reload Cached Resources //
        /////////////////////////////

        drawBufferRes.loadShadowMask();
        drawBufferRes.loadBrush();

        /////////////////////////////
        // Update Children Objects //
        /////////////////////////////

        m_caption->transform(captionTitleSelfcoordRect());
        if (m_content) m_content->transform(clientAreaSelfcoordRect());
    }

    void Window::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        ResizablePanel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);

        /////////////////////////////
        // Reload Cached Resources //
        /////////////////////////////

        decorativeBarRes.loadBrush();
    }

    void Window::onMouseMoveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseMoveHelper(e);

        auto& p = e.cursorPoint;

        if (!isPerformSpecialOperation())
        {
            if (button1Enabled)
            {
                if (!math_utils::isOverlappedExcludingRight(p, button1AbsoluteRect()))
                {
                    m_isButton1Hover = false;
                    m_isButton1Down = false;
                }
                else m_isButton1Hover = true;
            }
            if (button2Enabled)
            {
                if (!math_utils::isOverlapped(p, button2AbsoluteRect()))
                {
                    m_isButton2Hover = false;
                    m_isButton2Down = false;
                }
                else m_isButton2Hover = true;
            }
            if (button3Enabled)
            {
                if (!math_utils::isOverlappedExcludingLeft(p, button3AbsoluteRect()))
                {
                    m_isButton3Hover = false;
                    m_isButton3Down = false;
                }
                else m_isButton3Hover = true;
            }
        }
        else m_isButton1Hover = m_isButton2Hover = m_isButton3Hover = false;

        DraggablePanel::onMouseMoveWrapper(e);
        ResizablePanel::onMouseMoveWrapper(e);

        handleMouseMoveForRegisteredTabGroups(e);
    }

    void Window::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseLeaveHelper(e);
        ResizablePanel::onMouseLeaveWrapper(e);

        m_isButton1Hover = m_isButton1Down = false;
        m_isButton2Hover = m_isButton2Down = false;
        m_isButton3Hover = m_isButton3Down = false;
    }

    void Window::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Panel::onMouseButtonHelper(e);

        if (respondSetForegroundEvent)
        {
            if (e.state.leftDown() ||
                e.state.rightDown() ||
                e.state.middleDown())
            {
                moveTopmost();
            }
        }
        if (e.state.leftDown() || e.state.leftDblclk())
        {
            if (!isPerformSpecialOperation())
            {
                if (button1Enabled) m_isButton1Down = m_isButton1Hover;
                if (button2Enabled) m_isButton2Down = m_isButton2Hover;
                if (button3Enabled) m_isButton3Down = m_isButton3Hover;
            }
            else m_isButton1Down = m_isButton2Down = m_isButton3Down = false;
        }
        else if (e.state.leftUp())
        {
            if (m_isButton1Down)
            {
                setDisplayState(Minimized);
            }
            else if (m_isButton2Down)
            {
                if (m_displayState == Normal)
                {
                    setDisplayState(Maximized);
                }
                else setDisplayState(Normal);
            }
            else if (m_isButton3Down)
            {
                onClose();
            }
            m_isButton1Down = m_isButton2Down = m_isButton3Down = false;
        }
        DraggablePanel::onMouseButtonWrapper(e);
        ResizablePanel::onMouseButtonWrapper(e);

        handleMouseButtonForRegisteredTabGroups(e);
    }

    bool Window::isTriggerDraggingHelper(const Event::Point& p)
    {
        return math_utils::isInside(p, captionPanelAbsoluteRect()) &&
            !m_isButton1Hover && !m_isButton2Hover && !m_isButton3Hover;
    }
}
