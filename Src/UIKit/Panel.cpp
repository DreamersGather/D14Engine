#include "Common/Precompile.h"

#include "UIKit/Panel.h"

#include "Common/CppLangUtils/PointerCompare.h"
#include "Common/MathUtils/2D.h"

#include "UIKit/BitmapObject.h"
#include "UIKit/PlatformUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    Panel::Panel(
        const D2D1_RECT_F& rect,
        ComPtrParam<ID2D1Brush> brush,
        ComPtrParam<ID2D1Bitmap1> bitmap)
        :
        m_rect(rect),
        brush(brush),
        bitmap(bitmap)
    {
        ISortable<IDrawObject2D>::m_priority = 0;
        ISortable<Panel>::m_priority = 0;

        updateAbsoluteRect();
    }

    Panel::~Panel()
    {
        if (f_onDestroy) f_onDestroy(this);
    }

    void Panel::onInitializeFinish()
    {
        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;

        onChangeThemeStyleHelper(app->themeStyle());
        onChangeLangLocaleHelper(app->langLocale());
    }

    void Panel::registerDrawObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        if (m_drawObjects.find(uiobj) != m_drawObjects.end()) return;

        /////////////////////////
        // Update Draw Objects //
        /////////////////////////

        m_drawObjects.insert(uiobj);

        ///////////////////////
        // Update Priorities //
        ///////////////////////

        m_frontPriorities.drawObject = std::max
        (
            m_frontPriorities.drawObject,
            uiobj->drawObjectPriority()
        );
        m_backPriorities.drawObject = std::min
        (
            m_backPriorities.drawObject,
            uiobj->drawObjectPriority()
        );
    }

    void Panel::unregisterDrawObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;

        /////////////////////////
        // Update Draw Objects //
        /////////////////////////

        m_drawObjects.erase(uiobj);

        ///////////////////////
        // Update Priorities //
        ///////////////////////

        // There is no need to update the priorities here,
        // as newly registered objects will automatically
        // adapt to the existing priorities.
    }

    bool Panel::visible() const
    {
        return m_visible;
    }

    void Panel::setVisible(bool value)
    {
        m_visible = value;
    }

    bool Panel::enabled() const
    {
        return m_enabled;
    }

    void Panel::setEnabled(bool value)
    {
        m_enabled = value;

        updateAppEventReactability();
    }

    void Panel::setPrivateVisible(bool value)
    {
        m_privateVisible = value;
    }

    void Panel::setPrivateEnabled(bool value)
    {
        m_privateEnabled = value;

        updateAppEventReactability();
    }

    void Panel::updateAbsoluteRect()
    {
        // Special note: Do NOT use size()/position() methods here,
        // as their results are based on m_rect instead of m_absoluteRect.
        // Since it is m_absoluteRect that needs to be updated here,
        // it is better to directly extract its data with math_utils.

        auto originalSize = math_utils::size(m_absoluteRect);
        auto originalPosition = math_utils::leftTop(m_absoluteRect);

        /////////////////////
        // Update Geometry //
        /////////////////////

        if (!m_parent.expired())
        {
            auto parentPosition = m_parent.lock()->absolutePosition();
            m_absoluteRect = math_utils::offset(m_rect, parentPosition);
        }
        else m_absoluteRect = m_rect;

        /////////////////////
        // OnSize Callback //
        /////////////////////

        auto updatedSize = math_utils::size(m_absoluteRect);

        if (originalSize.width != updatedSize.width ||
            originalSize.height != updatedSize.height)
        {
            SizeEvent e = {};
            e.size = updatedSize;

            onSize(e);
        }
        /////////////////////
        // OnMove Callback //
        /////////////////////

        auto updatedPosition = math_utils::leftTop(m_absoluteRect);

        if (originalPosition.x != updatedPosition.x ||
            originalPosition.y != updatedPosition.y)
        {
            MoveEvent e = {};
            e.position = position();

            onMove(e);
        }
    }

    float Panel::width() const
    {
        return math_utils::width(m_rect);
    }

    float Panel::height() const
    {
        return math_utils::height(m_rect);
    }

    D2D1_SIZE_F Panel::size() const
    {
        return { width(), height() };
    }

    D2D1_SIZE_U Panel::pixelSize() const
    {
        return math_utils::roundu(platform_utils::scaledByDpi(size()));
    }

    void Panel::setSize(float width, float height)
    {
        m_rect.right = m_rect.left + std::clamp(width, minimalWidth(), maximalWidth());
        m_rect.bottom = m_rect.top + std::clamp(height, minimalHeight(), maximalHeight());
        updateAbsoluteRect();
    }

    void Panel::setSize(const D2D1_SIZE_F& size)
    {
        setSize(size.width, size.height);
    }

    float Panel::x() const
    {
        return m_rect.left;
    }

    float Panel::y() const
    {
        return m_rect.top;
    }

    D2D1_POINT_2F Panel::position() const
    {
        return { x(), y() };
    }

    float Panel::absoluteX() const
    {
        return m_absoluteRect.left;
    }

    float Panel::absoluteY() const
    {
        return m_absoluteRect.top;
    }

    D2D_POINT_2F Panel::absolutePosition() const
    {
        return { absoluteX(), absoluteY() };
    }

    void Panel::setPosition(float x, float y)
    {
        m_rect = math_utils::rect(x, y, width(), height());
        updateAbsoluteRect();
    }

    void Panel::setPosition(const D2D1_POINT_2F& point)
    {
        setPosition(point.x, point.y);
    }

    D2D1_RECT_F Panel::selfCoordRect() const
    {
        return { 0.0f, 0.0f, width(), height() };
    }

    const D2D1_RECT_F& Panel::relativeRect() const
    {
        return m_rect;
    }

    const D2D1_RECT_F& Panel::absoluteRect() const
    {
        return m_absoluteRect;
    }

    void Panel::transform(float x, float y, float width, float height)
    {
        if (width >= minimalWidth() && width <= maximalWidth() &&
            height >= minimalHeight() && height <= maximalHeight())
        {
            m_rect = math_utils::rect(x, y, width, height);
            updateAbsoluteRect();
        }
    }

    void Panel::transform(const D2D1_RECT_F& rect)
    {
        transform(rect.left, rect.top, math_utils::width(rect), math_utils::height(rect));
    }

    D2D1_POINT_2F Panel::selfCoordToRelative(const D2D1_POINT_2F& p) const
    {
        return { p.x + m_rect.left, p.y + m_rect.top };
    }

    D2D1_RECT_F Panel::selfCoordToRelative(const D2D1_RECT_F& rect) const
    {
        auto leftTop = selfCoordToRelative(D2D1_POINT_2F{ rect.left, rect.top });
        auto rightBottom = selfCoordToRelative(D2D1_POINT_2F{ rect.right, rect.bottom });

        return { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    }

    D2D1_POINT_2F Panel::selfCoordToAbsolute(const D2D1_POINT_2F& p) const
    {
        return { p.x + m_absoluteRect.left, p.y + m_absoluteRect.top };
    }

    D2D1_RECT_F Panel::selfCoordToAbsolute(const D2D1_RECT_F& rect) const
    {
        auto leftTop = selfCoordToAbsolute(D2D1_POINT_2F{ rect.left, rect.top });
        auto rightBottom = selfCoordToAbsolute(D2D1_POINT_2F{ rect.right, rect.bottom });

        return { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    }

    D2D1_POINT_2F Panel::relativeToSelfCoord(const D2D1_POINT_2F& p) const
    {
        return { p.x - m_rect.left, p.y - m_rect.top };
    }

    D2D1_RECT_F Panel::relativeToSelfCoord(const D2D1_RECT_F& rect) const
    {
        auto leftTop = relativeToSelfCoord(D2D1_POINT_2F{ rect.left, rect.top });
        auto rightBottom = relativeToSelfCoord(D2D1_POINT_2F{ rect.right, rect.bottom });

        return { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    }

    D2D1_POINT_2F Panel::relativeToAbsolute(const D2D1_POINT_2F& p) const
    {
        return { p.x - m_rect.left + m_absoluteRect.left, p.y - m_rect.top + m_absoluteRect.top };
    }

    D2D1_RECT_F Panel::relativeToAbsolute(const D2D1_RECT_F& rect) const
    {
        auto leftTop = relativeToAbsolute(D2D1_POINT_2F{ rect.left, rect.top });
        auto rightBottom = relativeToAbsolute(D2D1_POINT_2F{ rect.right, rect.bottom });

        return { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    }

    D2D1_POINT_2F Panel::absoluteToSelfCoord(const D2D1_POINT_2F& p) const
    {
        return { p.x - m_absoluteRect.left, p.y - m_absoluteRect.top };
    }

    D2D1_RECT_F Panel::absoluteToSelfCoord(const D2D1_RECT_F& rect) const
    {
        auto leftTop = absoluteToSelfCoord(D2D1_POINT_2F{ rect.left, rect.top });
        auto rightBottom = absoluteToSelfCoord(D2D1_POINT_2F{ rect.right, rect.bottom });

        return { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    }

    D2D1_POINT_2F Panel::absoluteToRelative(const D2D1_POINT_2F& p) const
    {
        return { p.x + m_rect.left - m_absoluteRect.left, p.y + m_rect.top - m_absoluteRect.top };
    }

    D2D1_RECT_F Panel::absoluteToRelative(const D2D1_RECT_F& rect) const
    {
        auto leftTop = absoluteToRelative(D2D1_POINT_2F{ rect.left, rect.top });
        auto rightBottom = absoluteToRelative(D2D1_POINT_2F{ rect.right, rect.bottom });

        return { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    }

    bool Panel::isPlayAnimation() const
    {
        return m_isPlayAnimation;
    }

    void Panel::increaseAnimationCount()
    {
        THROW_IF_NULL(Application::g_app);

        if (!m_isPlayAnimation)
        {
            m_isPlayAnimation = true;
            Application::g_app->increaseAnimationCount();
        }
    }

    void Panel::decreaseAnimationCount()
    {
        THROW_IF_NULL(Application::g_app);

        if (m_isPlayAnimation)
        {
            m_isPlayAnimation = false;
            Application::g_app->decreaseAnimationCount();
        }
    }

    void Panel::updateChildrenObjects(Renderer* rndr)
    {
        for (auto& drawobj : m_drawObjects)
        {
            if (drawobj->isD2d1ObjectVisible())
            {
                drawobj->onRendererUpdateObject2D(rndr);
            }
        }
    }

    void Panel::drawChildrenLayers(Renderer* rndr)
    {
        for (auto& drawobj : m_drawObjects)
        {
            if (drawobj->isD2d1ObjectVisible())
            {
                drawobj->onRendererDrawD2d1Layer(rndr);
            }
        }
    }

    void Panel::drawBackground(Renderer* rndr)
    {
        if (brush)
        {
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ { m_absoluteRect, roundRadiusX, roundRadiusY },
            /* brush       */ brush.Get()
            );
        }
        if (bitmap)
        {
            D2D1_INTERPOLATION_MODE mode = {};
            if (bitmapProperty.interpolationMode.has_value())
            {
                mode = bitmapProperty.interpolationMode.value();
            }
            else mode = BitmapObject::g_interpolationMode;

            rndr->d2d1DeviceContext()->DrawBitmap
            (
            /* bitmap               */ bitmap.Get(),
            /* destinationRectangle */ m_absoluteRect,
            /* opacity              */ bitmapProperty.opacity,
            /* interpolationMode    */ mode
            );
        }
    }

    void Panel::drawChildrenObjects(Renderer* rndr)
    {
        for (auto& drawobj : m_drawObjects)
        {
            if (drawobj->isD2d1ObjectVisible())
            {
                drawobj->onRendererDrawD2d1Object(rndr);
            }
        }
    }

    void Panel::drawD2d1ObjectPreceding(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    void Panel::drawD2d1ObjectPosterior(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    const WeakPtr<Panel>& Panel::parent() const
    {
        return m_parent;
    }

    void Panel::setParent(ShrdPtrRefer<Panel> uiobj)
    {
        if (!uiobj)
        {
            if (!m_parent.expired())
            {
                m_parent.lock()->removeUIObject(shared_from_this());
                m_parent.reset();
            }
            return;
        }
        if (!m_parent.expired())
        {
            auto pp = m_parent.lock();
            if (!cpp_lang_utils::isMostDerivedEqual(pp, uiobj))
            {
                // The ref-count of this may be 0 after removed from parent,
                // so we must retain a temporary ptr to avoid sudden release.
                auto temporaryLocked = shared_from_this();

                pp->removeUIObject(temporaryLocked);
                uiobj->addUIObject(temporaryLocked);
            }
        }
        else uiobj->addUIObject(shared_from_this());
    }

    const Panel::ChildObjectSet& Panel::children() const
    {
        return m_children;
    }

    void Panel::registerUIEvents(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        if (m_children.find(uiobj) != m_children.end()) return;

        ///////////////////////
        // Update UI Objects //
        ///////////////////////

        m_children.insert(uiobj);

        ///////////////////////
        // Update Priorities //
        ///////////////////////

        m_frontPriorities.uiObject = std::min
        (
            m_frontPriorities.uiObject,
            uiobj->uiObjectPriority()
        );
        m_backPriorities.uiObject = std::max
        (
            m_backPriorities.uiObject,
            uiobj->uiObjectPriority()
        );
        ///////////////////
        // Update Parent //
        ///////////////////

        if (!uiobj->m_parent.expired())
        {
            uiobj->m_parent.lock()->removeUIObject(uiobj);
        }
        uiobj->m_parent = shared_from_this();

        /////////////////////
        // Update Geometry //
        /////////////////////

        // Changing parent should not trigger size/move-event,
        // so we only need to update the relative rect here.
        uiobj->m_rect = math_utils::offset
        (
        /* rect   */ uiobj->m_absoluteRect,
        /* offset */ math_utils::minus(absolutePosition())
        );
    }

    void Panel::unregisterUIEvents(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;

        ///////////////////////
        // Update UI Objects //
        ///////////////////////

        m_children.erase(uiobj);

        ///////////////////////
        // Update Priorities //
        ///////////////////////

        // There is no need to update the priorities here,
        // as newly registered objects will automatically
        // adapt to the existing priorities.

        auto pParent = uiobj->m_parent.lock();
        auto pThis = shared_from_this();

        if (cpp_lang_utils::isMostDerivedEqual(pParent, pThis))
        {
            ///////////////////
            // Update Parent //
            ///////////////////

            uiobj->m_parent.reset();

            /////////////////////
            // Update Geometry //
            /////////////////////

            uiobj->m_rect = uiobj->m_absoluteRect;
        }
    }

    void Panel::addUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        registerDrawObject(uiobj);
        registerUIEvents(uiobj);
    }

    void Panel::removeUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        unregisterDrawObject(uiobj);
        unregisterUIEvents(uiobj);
    }

    void Panel::clearAddedUIObjects()
    {
        for (auto& child : m_children)
        {
            child->m_parent.reset();
            child->m_rect = child->m_absoluteRect;
        }
        m_drawObjects.clear();
        m_children.clear();

        m_frontPriorities = {};
        m_backPriorities = {};
    }

    void Panel::pinUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        m_pinnedChildren.insert(uiobj);
    }

    void Panel::unpinUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        m_pinnedChildren.erase(uiobj);
    }

    void Panel::clearPinnedUIObjects()
    {
        m_pinnedChildren.clear();
    }

    int Panel::drawObjectPriority() const
    {
        return ISortable<IDrawObject2D>::priority();
    }

    void Panel::setDrawObjectPriority(int value)
    {
        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;

        if (m_parent.expired())
        {
            auto& drawobjs = app->drawObjects();
            if (drawobjs.find(shared_from_this()) != drawobjs.end())
            {
                app->unregisterDrawObject(shared_from_this());
                ISortable<IDrawObject2D>::m_priority = value;
                app->registerDrawObject(shared_from_this());
            }
        }
        else // Managed by another object.
        {
            auto parentPtr = m_parent.lock();

            parentPtr->unregisterDrawObject(shared_from_this());
            ISortable<IDrawObject2D>::m_priority = value;
            parentPtr->registerDrawObject(shared_from_this());
        }
    }

    int Panel::uiObjectPriority() const
    {
        return ISortable<Panel>::priority();
    }

    void Panel::setUIObjectPriority(int value)
    {
        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;

        if (m_parent.expired())
        {
            auto& uiobjs = app->uiObjects();
            if (uiobjs.find(shared_from_this()) != uiobjs.end())
            {
                app->unregisterUIEvents(shared_from_this());
                ISortable<Panel>::m_priority = value;
                app->registerUIEvents(shared_from_this());
            }
        }
        else // Managed by another object.
        {
            auto parentPtr = m_parent.lock();

            parentPtr->unregisterUIEvents(shared_from_this());
            ISortable<Panel>::m_priority = value;
            parentPtr->registerUIEvents(shared_from_this());
        }
    }

    const Panel::PriorityGroup& Panel::frontPriorities() const
    {
        return m_frontPriorities;
    }

    const Panel::PriorityGroup& Panel::backPriorities() const
    {
        return m_backPriorities;
    }

    void Panel::bringToFront()
    {
        THROW_IF_NULL(Application::g_app);

        if (m_parent.expired())
        {
            Application::g_app->bringRootObjectToFront(this);
        }
        else // Managed by another object.
        {
            m_parent.lock()->bringChildObjectToFront(this);
        }
    }

    void Panel::sendToBack()
    {
        THROW_IF_NULL(Application::g_app);

        if (m_parent.expired())
        {
            Application::g_app->sendRootObjectToBack(this);
        }
        else // Managed by another object.
        {
            m_parent.lock()->sendChildObjectToBack(this);
        }
    }

    void Panel::bringChildObjectToFront(Panel* uiobj)
    {
        uiobj->setDrawObjectPriority(++m_frontPriorities.drawObject);
        uiobj->setUIObjectPriority(--m_frontPriorities.uiObject);
    }

    void Panel::sendChildObjectToBack(Panel* uiobj)
    {
        uiobj->setDrawObjectPriority(++m_backPriorities.drawObject);
        uiobj->setUIObjectPriority(--m_backPriorities.uiObject);
    }

    void Panel::reorderAbovePeerObject(Panel* uiobj)
    {
        setDrawObjectPriority(uiobj->drawObjectPriority() + 1);
        setUIObjectPriority(uiobj->uiObjectPriority() - 1);
    }

    void Panel::reorderBelowPeerObject(Panel* uiobj)
    {
        setDrawObjectPriority(uiobj->drawObjectPriority() - 1);
        setUIObjectPriority(uiobj->uiObjectPriority() + 1);
    }

    void Panel::ApplicationEventReactability::set(bool value)
    {
        mouse.enter = mouse.move = mouse.leave =
        mouse.button = mouse.wheel = keyboard = hitTest = value;
    }

    void Panel::updateAppEventReactability()
    {
        appEventReactability.set(m_enabled && m_privateEnabled);
    }

    bool Panel::release()
    {
        THROW_IF_NULL(Application::g_app);

        auto& app = Application::g_app;

        app->dx12Renderer()->waitGpuCommand();

        bool isReleased = true;

        // After Step 1, the ref-count of this may be 0 if it is released
        // successfully, so we must retain a temporary ptr to make sure the
        // callback in Step 2 could be triggered correctly (if it exists).
        auto temporaryLocked = shared_from_this();

        /////////////////////////////////
        // Step 1 - Update Collections //
        /////////////////////////////////

        if (m_parent.expired())
        {
            app->removeUIObject(shared_from_this());
        }
        else // Managed by another object.
        {
            isReleased = m_parent.lock()->releaseUIObject(temporaryLocked);
        }
        ///////////////////////////////
        // Step 2 - Trigger Callback //
        ///////////////////////////////

        if (isReleased && f_onRelease) f_onRelease(this);

        return isReleased;
    }

    bool Panel::releaseUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        THROW_IF_NULL(Application::g_app);

        Application::g_app->dx12Renderer()->waitGpuCommand();

        if (f_onReleaseUIObject)
        {
            return f_onReleaseUIObject(this, uiobj);
        }
        else return releaseUIObjectHelper(uiobj);
    }

    bool Panel::isHit(const Event::Point& p) const
    {
        if (f_isHit)
        {
            return f_isHit(this, p);
        }
        else return isHitHelper(p);
    }

    void Panel::onGetMouseFocus()
    {
        onGetMouseFocusHelper();

        if (f_onGetMouseFocus) f_onGetMouseFocus(this);
    }

    void Panel::onGetKeyboardFocus()
    {
        onGetKeyboardFocusHelper();

        if (f_onGetKeyboardFocus) f_onGetKeyboardFocus(this);
    }

    void Panel::onLoseMouseFocus()
    {
        onLoseMouseFocusHelper();

        if (f_onLoseMouseFocus) f_onLoseMouseFocus(this);
    }

    void Panel::onLoseKeyboardFocus()
    {
        onLoseKeyboardFocusHelper();

        if (f_onLoseKeyboardFocus) f_onLoseKeyboardFocus(this);
    }

    bool Panel::holdMouseFocus() const
    {
        THROW_IF_NULL(Application::g_app);

        auto focus = (size_t)Application::FocusType::Mouse;
        auto& uiobj = Application::g_app->m_focusedUIObjects[focus];

        return cpp_lang_utils::isMostDerivedEqual(uiobj.lock(), shared_from_this());
    }

    bool Panel::holdKeyboardFocus() const
    {
        THROW_IF_NULL(Application::g_app);

        auto focus = (size_t)Application::FocusType::Keyboard;
        auto& uiobj = Application::g_app->m_focusedUIObjects[focus];

        return cpp_lang_utils::isMostDerivedEqual(uiobj.lock(), shared_from_this());
    }

    void Panel::onSize(SizeEvent& e)
    {
        onSizeHelper(e);

        if (f_onSize) f_onSize(this, e);
    }

    void Panel::onParentSize(SizeEvent& e)
    {
        onParentSizeHelper(e);

        if (f_onParentSize) f_onParentSize(this, e);
    }

    float Panel::minimalWidth() const
    {
        return minimalWidthHint.has_value() ? minimalWidthHint.value() : 0.0f;
    }

    float Panel::minimalHeight() const
    {
        return minimalHeightHint.has_value() ? minimalHeightHint.value() : 0.0f;
    }

    D2D1_SIZE_F Panel::minimalSize() const
    {
        return { minimalWidth(), minimalHeight() };
    }

    float Panel::maximalWidth() const
    {
        return maximalWidthHint.has_value() ? maximalWidthHint.value() : FLT_MAX;
    }

    float Panel::maximalHeight() const
    {
        return maximalHeightHint.has_value() ? maximalHeightHint.value() : FLT_MAX;
    }

    D2D1_SIZE_F Panel::maximalSize() const
    {
        return { maximalWidth(), maximalHeight() };
    }

    void Panel::onMove(MoveEvent& e)
    {
        onMoveHelper(e);

        if (f_onMove) f_onMove(this, e);
    }

    void Panel::onParentMove(MoveEvent& e)
    {
        onParentMoveHelper(e);

        if (f_onParentMove) f_onParentMove(this, e);
    }

    void Panel::onMouseEnter(MouseMoveEvent& e)
    {
        onMouseEnterHelper(e);

        if (f_onMouseEnter) f_onMouseEnter(this, e);
    }

    void Panel::onMouseMove(MouseMoveEvent& e)
    {
        onMouseMoveHelper(e);

        if (f_onMouseMove) f_onMouseMove(this, e);
    }

    void Panel::onMouseLeave(MouseMoveEvent& e)
    {
        onMouseLeaveHelper(e);

        if (f_onMouseLeave) f_onMouseLeave(this, e);
    }

    void Panel::onMouseButton(MouseButtonEvent& e)
    {
        onMouseButtonHelper(e);

        if (f_onMouseButton) f_onMouseButton(this, e);
    }

    void Panel::onMouseWheel(MouseWheelEvent& e)
    {
        onMouseWheelHelper(e);

        if (f_onMouseWheel) f_onMouseWheel(this, e);
    }

    void Panel::onKeyboard(KeyboardEvent& e)
    {
        onKeyboardHelper(e);

        if (f_onKeyboard) f_onKeyboard(this, e);
    }

    void Panel::onChangeThemeStyle(const ThemeStyle& style)
    {
        onChangeThemeStyleHelper(style);

        if (f_onChangeThemeStyle) f_onChangeThemeStyle(this, style);
    }

    void Panel::onChangeLangLocale(WstrRefer codeName)
    {
        onChangeLangLocaleHelper(codeName);

        if (f_onChangeLangLocale) f_onChangeLangLocale(this, codeName);
    }

    bool Panel::isD2d1ObjectVisible() const
    {
        return m_visible && m_privateVisible;
    }

    void Panel::setD2d1ObjectVisible(bool value)
    {
        m_visible = value; // no for private
    }

    void Panel::onRendererUpdateObject2D(Renderer* rndr)
    {
        if (f_onRendererUpdateObject2DBefore)
        {
            f_onRendererUpdateObject2DBefore(this, rndr);
        }
        updateChildrenObjects(rndr);

        onRendererUpdateObject2DHelper(rndr);

        if (f_onRendererUpdateObject2DAfter)
        {
            f_onRendererUpdateObject2DAfter(this, rndr);
        }
    }

    void Panel::onRendererDrawD2d1Layer(Renderer* rndr)
    {
        if (f_onRendererDrawD2d1LayerBefore)
        {
            f_onRendererDrawD2d1LayerBefore(this, rndr);
        }
        drawChildrenLayers(rndr);

        onRendererDrawD2d1LayerHelper(rndr);

        if (f_onRendererDrawD2d1LayerAfter)
        {
            f_onRendererDrawD2d1LayerAfter(this, rndr);
        }
    }

    void Panel::onRendererDrawD2d1Object(Renderer* rndr)
    {
        if (f_onRendererDrawD2d1ObjectBefore)
        {
            f_onRendererDrawD2d1ObjectBefore(this, rndr);
        }
        drawD2d1ObjectPreceding(rndr);

        onRendererDrawD2d1ObjectHelper(rndr);

        drawChildrenObjects(rndr);

        drawD2d1ObjectPosterior(rndr);

        if (f_onRendererDrawD2d1ObjectAfter)
        {
            f_onRendererDrawD2d1ObjectAfter(this, rndr);
        }
    }

    bool Panel::releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj)
    {
        removeUIObject(uiobj); return true;
    }

    bool Panel::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isOverlapped(p, m_absoluteRect);
    }

    void Panel::onGetMouseFocusHelper()
    {
        // This method intentionally left blank.
    }

    void Panel::onGetKeyboardFocusHelper()
    {
        // This method intentionally left blank.
    }

    void Panel::onLoseMouseFocusHelper()
    {
        // This method intentionally left blank.
    }

    void Panel::onLoseKeyboardFocusHelper()
    {
        // This method intentionally left blank.
    }

    void Panel::onSizeHelper(SizeEvent& e)
    {
        for (auto& child : m_children)
        {
            child->onParentSize(e);
        }
    }

    void Panel::onParentSizeHelper(SizeEvent& e)
    {
        // This method intentionally left blank.
    }

    void Panel::onMoveHelper(MoveEvent& e)
    {
        MoveEvent me = {};
        // Always (0,0) in the self coordinate.
        me.position = { 0.0f, 0.0f };

        for (auto& child : m_children)
        {
            child->onParentMove(me);
        }
    }

    void Panel::onParentMoveHelper(MoveEvent& e)
    {
        updateAbsoluteRect();

        MoveEvent me = {};
        me.position = position();

        onMove(me);
    }

    void Panel::onMouseEnterHelper(MouseMoveEvent& e)
    {
        // This method intentionally left blank.
    }

    void Panel::onMouseMoveHelper(MouseMoveEvent& e)
    {
        ISortable<Panel>::foreach(m_pinnedChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.move)
            {
                child->onMouseMove(e);
            }
            return child->appEventTransparency.mouse.move;
        });
        if (!enableChildrenMouseMoveEvent) return;

        ChildObjectTempSet hitChildren = {};

        for (auto& child : m_children)
        {
            if (child->appEventReactability.hitTest && child->isHit(e.cursorPoint))
            {
                hitChildren.insert(child);
            }
        }
        if (forceSingleMouseEnterLeaveEvent)
        {
            WeakPtr<Panel> enterCandidate = {}, leaveCandidate = {};
            if (!hitChildren.empty())
            {
                enterCandidate = *hitChildren.begin();
            }
            if (!m_hitChildren.empty())
            {
                leaveCandidate = *m_hitChildren.begin();
            }
            if (!cpp_lang_utils::isMostDerivedEqual(enterCandidate, leaveCandidate))
            {
                if (!enterCandidate.expired())
                {
                    auto candidate = enterCandidate.lock();
                    if (candidate->appEventReactability.mouse.enter)
                    {
                        candidate->onMouseEnter(e);
                    }
                }
                if (!leaveCandidate.expired())
                {
                    auto candidate = leaveCandidate.lock();
                    if (candidate->appEventReactability.mouse.leave)
                    {
                        candidate->onMouseLeave(e);
                    }
                }
            }
        }
        else // trigger multiple mouse-enter-leave events
        {
            ISortable<Panel>::foreach(hitChildren, [&](ShrdPtrRefer<Panel> child)
            {
                // Moved in just now, trigger onMouseEnter event.
                if (m_hitChildren.find(child) == m_hitChildren.end())
                {
                    if (child->appEventReactability.mouse.enter)
                    {
                        child->onMouseEnter(e);
                    }
                    return child->appEventTransparency.mouse.enter;
                }
                return true;
            });
            ISortable<Panel>::foreach(m_hitChildren, [&](ShrdPtrRefer<Panel> child)
            {
                // Moved out just now, trigger onMouseLeave event.
                if (hitChildren.find(child) == hitChildren.end())
                {
                    if (child->appEventReactability.mouse.leave)
                    {
                        child->onMouseLeave(e);
                    }
                    return child->appEventTransparency.mouse.leave;
                }
                return true;
            });
        }
        m_hitChildren = std::move(hitChildren);

        ISortable<Panel>::foreach(m_hitChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.move)
            {
                child->onMouseMove(e);
            }
            return child->appEventTransparency.mouse.move;
        });
    }

    void Panel::onMouseLeaveHelper(MouseMoveEvent& e)
    {
        ISortable<Panel>::foreach(m_hitChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.leave)
            {
                child->onMouseLeave(e);
            }
            return child->appEventTransparency.mouse.leave;
        });
        m_hitChildren.clear();
    }

    void Panel::onMouseButtonHelper(MouseButtonEvent& e)
    {
        ISortable<Panel>::foreach(m_pinnedChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.button)
            {
                child->onMouseButton(e);
            }
            return child->appEventTransparency.mouse.button;
        });
        ISortable<Panel>::foreach(m_hitChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.button)
            {
                child->onMouseButton(e);
            }
            return child->appEventTransparency.mouse.button;
        });
    }

    void Panel::onMouseWheelHelper(MouseWheelEvent& e)
    {
        ISortable<Panel>::foreach(m_pinnedChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.wheel)
            {
                child->onMouseWheel(e);
            }
            return child->appEventTransparency.mouse.wheel;
        });
        ISortable<Panel>::foreach(m_hitChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.mouse.wheel)
            {
                child->onMouseWheel(e);
            }
            return child->appEventTransparency.mouse.wheel;
        });
    }

    void Panel::onKeyboardHelper(KeyboardEvent& e)
    {
        ISortable<Panel>::foreach(m_pinnedChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.keyboard)
            {
                child->onKeyboard(e);
            }
            return child->appEventTransparency.keyboard;
        });
        ISortable<Panel>::foreach(m_hitChildren, [&](ShrdPtrRefer<Panel> child)
        {
            if (child->appEventReactability.keyboard)
            {
                child->onKeyboard(e);
            }
            return child->appEventTransparency.keyboard;
        });
    }

    void Panel::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        for (auto& child : m_children)
        {
            if (child->enableChangeThemeStyleUpdate)
            {
                child->onChangeThemeStyle(style);
            }
        }
    }

    void Panel::onChangeLangLocaleHelper(WstrRefer codeName)
    {
        for (auto& child : m_children)
        {
            if (child->enableChangeLangLocaleUpdate)
            {
                child->onChangeLangLocale(codeName);
            }
        }
    }

    void Panel::onRendererUpdateObject2DHelper(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    void Panel::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    void Panel::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        drawBackground(rndr);
    }
}
