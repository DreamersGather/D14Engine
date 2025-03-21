﻿#pragma once

#include "Common/Precompile.h"

#include "Renderer/Interfaces/IDrawObject2D.h"
#include "Renderer/Renderer.h"

#include "UIKit/Appearances/Appearance.h"
#include "UIKit/Event.h"

namespace d14engine::uikit
{
    struct Application;
    struct ComboBox;
    struct PopupMenu;
    struct Slider;
    struct TabGroup;
    struct TreeView;

    template<typename Item_T>
    struct WaterfallView;

    struct Panel : cpp_lang_utils::NonCopyable, renderer::IDrawObject2D,
                   std::enable_shared_from_this<Panel>, ISortable<Panel>
    {
        friend Application;
        friend ComboBox;
        friend PopupMenu;
        friend Slider;
        friend TabGroup;
        friend TreeView;

        // Due to the syntax issue of friend template,
        // the keyword (struct) here cannot be omitted.
        template<typename Item_T>
        friend struct WaterfallView;

        Panel(
            const D2D1_RECT_F& rect = {},
            ComPtrParam<ID2D1Brush> brush = nullptr,
            ComPtrParam<ID2D1Bitmap1> bitmap = nullptr);

        virtual ~Panel();

        Function<void(Panel*)> f_onDestroy = {};

        //------------------------------------------------------------------
        // Some initialization processes depend on special operations,
        // and thus they cannot be completed in the constructor of Panel:
        //------------------------------------------------------------------
        // 1. virtual methods:
        //    The vtbl is not fully initialized at this time.
        // 
        // 2. enable_shared_from_this:
        //    Some methods depend on it (such as addUIObject).
        // 
        //------------------------------------------------------------------
        // These initializations should be moved into onInitializeFinish.
        //------------------------------------------------------------------
        virtual void onInitializeFinish();

        // Return whether the UI object is released successfully.

        bool release();

        Function<void(Panel*)> f_onRelease = {};

        bool releaseUIObject(ShrdPtrRefer<Panel> uiobj);

        Function<bool(Panel*, ShrdPtrRefer<Panel>)> f_onReleaseUIObject = {};

    public:
        bool isD2d1ObjectVisible() const override;

        void setD2d1ObjectVisible(bool value) override;

        void onRendererUpdateObject2D(renderer::Renderer* rndr) override;

        Function<void(Panel*, renderer::Renderer*)>
            f_onRendererUpdateObject2DBefore = {},
            f_onRendererUpdateObject2DAfter = {};

        void onRendererDrawD2d1Layer(renderer::Renderer* rndr) override;

        Function<void(Panel*, renderer::Renderer*)>
            f_onRendererDrawD2d1LayerBefore = {},
            f_onRendererDrawD2d1LayerAfter = {};

        void onRendererDrawD2d1Object(renderer::Renderer* rndr) override;

        Function<void(Panel*, renderer::Renderer*)>
            f_onRendererDrawD2d1ObjectBefore = {},
            f_onRendererDrawD2d1ObjectAfter = {};

    protected:
        virtual void onRendererUpdateObject2DHelper(renderer::Renderer* rndr);
        virtual void onRendererDrawD2d1LayerHelper(renderer::Renderer* rndr);
        virtual void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr);

    protected:
        bool m_isPlayAnimation = false;

    public:
        bool isPlayAnimation() const;

        void increaseAnimationCount();
        void decreaseAnimationCount();

    public:
        template<bool presetBoolean>
        struct ApplicationEventGroup
        {
            struct Mouse
            {
                bool enter = presetBoolean;
                bool move = presetBoolean;
                bool leave = presetBoolean;
                bool button = presetBoolean;
                bool wheel = presetBoolean;
            }
            mouse = {};

            bool keyboard = presetBoolean;

            virtual void setFlag(bool value)
            {
                mouse.enter = value;
                mouse.move = value;
                mouse.leave = value;
                mouse.button = value;
                mouse.wheel = value;

                keyboard = value;
            }
        };

        // We want to find a word for describing whether the panel is able to
        // react or respond to the specific app-event, but unfortunately none.
        //
        // After consulting Limiao, we are determinded to use "reactability",
        // since we think it is very consistent with what we want to express.

        struct ApplicationEventReactability : ApplicationEventGroup<true>
        {
            bool hitTest = true;

            struct Focus
            {
                bool get = false;
                bool lose = true;
            }
            focus = {};

            void setFlag(bool value) override;
        }
        appEventReactability = {};

        using ApplicationEventTransparency = ApplicationEventGroup<false>;

        ApplicationEventTransparency appEventTransparency = {};

    public:
        bool isHit(const Event::Point& p) const;

        Function<bool(const Panel*, const Event::Point&)> f_isHit = {};

        // The derived class can choose whether to prevent the user-defined
        // minimal/maximal hints from working by overriding these series of
        // methods in specific way.
        //
        // If the derived class keeps the original method definition, then
        // the user-defined hints are returned for these series of methods.

        virtual float minimalWidth() const;
        virtual float minimalHeight() const;

        Optional<float> minimalWidthHint = {};
        Optional<float> minimalHeightHint = {};

        D2D1_SIZE_F minimalSize() const;

        virtual float maximalWidth() const;
        virtual float maximalHeight() const;

        Optional<float> maximalWidthHint = {};
        Optional<float> maximalHeightHint = {};

        D2D1_SIZE_F maximalSize() const;

        void onSize(SizeEvent& e);

        Function<void(Panel*, SizeEvent&)> f_onSize = {};

        void onParentSize(SizeEvent& e);

        Function<void(Panel*, SizeEvent&)> f_onParentSize = {};

        void onMove(MoveEvent& e);

        Function<void(Panel*, MoveEvent&)> f_onMove = {};

        void onParentMove(MoveEvent& e);

        Function<void(Panel*, MoveEvent&)> f_onParentMove = {};

        using ThemeStyle = appearance::Appearance::ThemeStyle;

        void onChangeThemeStyle(const ThemeStyle& style);

        Function<void(Panel*, const ThemeStyle&)> f_onChangeThemeStyle = {};

        void onChangeLangLocale(WstrRefer codeName);

        Function<void(Panel*, WstrRefer)> f_onChangeLangLocale = {};

        void onGetFocus();

        Function<void(Panel*)> f_onGetFocus = {};

        void onLoseFocus();

        Function<void(Panel*)> f_onLoseFocus = {};

        bool isFocused() const;
        bool forceGlobalExclusiveFocusing = false;

        void onMouseEnter(MouseMoveEvent& e);

        Function<void(Panel*, MouseMoveEvent&)> f_onMouseEnter = {};

        void onMouseMove(MouseMoveEvent& e);

        Function<void(Panel*, MouseMoveEvent&)> f_onMouseMove = {};

        void onMouseLeave(MouseMoveEvent& e);

        Function<void(Panel*, MouseMoveEvent&)> f_onMouseLeave = {};

        bool forceSingleMouseEnterLeaveEvent = true;
        bool forceTriggerChildrenMouseLeaveEvent = true;

        void onMouseButton(MouseButtonEvent& e);

        Function<void(Panel*, MouseButtonEvent&)> f_onMouseButton = {};

        void onMouseWheel(MouseWheelEvent& e);

        Function<void(Panel*, MouseWheelEvent&)> f_onMouseWheel = {};

        void onKeyboard(KeyboardEvent& e);

        Function<void(Panel*, KeyboardEvent&)> f_onKeyboard = {};

    protected:
        virtual bool isHitHelper(const Event::Point& p) const;
        virtual bool releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj);

        // Introduce onXxxHelper to solve the inheritance conflicts of
        // the "override", "before" and "after" event callback lambdas.
        //
        // *--------*-----------------------------------------*-------------------------------------*
        // | Class  | onSize                                  | onSizeHelper                        |
        // *--------*-----------------------------------------*-------------------------------------*
        // | Panel  | "before"; Panel::onSizeHelper; "after"  | Panel's works                       |
        // *--------*-----------------------------------------*-------------------------------------*
        // | Window | "before"; Window::onSizeHelper; "after" | Panel::onSizeHelper; Window's works |
        // *--------*-----------------------------------------*-------------------------------------*
        //
        // To sum up, do the actual works in onXxxHelper methods and wrap them into onXxx methods.

        virtual void onSizeHelper(SizeEvent& e);
        virtual void onParentSizeHelper(SizeEvent& e);
        virtual void onMoveHelper(MoveEvent& e);
        virtual void onParentMoveHelper(MoveEvent& e);
        virtual void onChangeThemeStyleHelper(const ThemeStyle& style);
        virtual void onChangeLangLocaleHelper(WstrRefer codeName);
        virtual void onGetFocusHelper();
        virtual void onLoseFocusHelper();
        virtual void onMouseEnterHelper(MouseMoveEvent& e);
        virtual void onMouseMoveHelper(MouseMoveEvent& e);
        virtual void onMouseLeaveHelper(MouseMoveEvent& e);
        virtual void onMouseButtonHelper(MouseButtonEvent& e);
        virtual void onMouseWheelHelper(MouseWheelEvent& e);
        virtual void onKeyboardHelper(KeyboardEvent& e);

        bool m_skipChangeChildrenThemeStyle = false;
        bool m_skipChangeChildrenLangLocale = false;
        bool m_skipDeliverNextMouseMoveEventToChildren = false;
        bool m_skipUpdateChildrenHitStateInMouseMoveEvent = false;

    public:
        int d2d1ObjectPriority() const;
        void setD2d1ObjectPriority(int value);

        int uiObjectPriority() const;
        void setUIObjectPriority(int value);

    protected:
        struct TopmostPriority
        {
            int d2d1Object = 0;
            int uiObject = 0;
        }
        m_topmostPriority = {};

    public:
        const TopmostPriority& topmostPriority() const;

        void moveTopmost();

        void moveChildObjectTopmost(Panel* uiobj);

        void moveAbovePeerObject(Panel* uiobj);
        void moveBelowPeerObject(Panel* uiobj);

    public:
        ComPtr<ID2D1Brush> brush = {};

        ComPtr<ID2D1Bitmap1> bitmap = {};
        struct BitmapProperty
        {
            float opacity = 1.0f;
            Optional<D2D1_INTERPOLATION_MODE> interpolationMode = {};
        }
        bitmapProperty = {};

        float roundRadiusX = 0.0f, roundRadiusY = 0.0f;

    protected:
        void updateChildrenObjects(renderer::Renderer* rndr);

        bool m_takeOverChildrenUpdating = false;

        void drawChildrenLayers(renderer::Renderer* rndr);

        void drawBackground(renderer::Renderer* rndr);

        void drawChildrenObjects(renderer::Renderer* rndr);

        bool m_takeOverChildrenDrawing = false;

    public:
        virtual void drawD2d1ObjectPreceding(renderer::Renderer* rndr);

        bool skipDrawPrecedingObjects = false;

        virtual void drawD2d1ObjectPosterior(renderer::Renderer* rndr);

        bool skipDrawPosteriorObjects = false;

    protected:
        bool m_visible = true;
        bool m_enabled = true;

        // Consider a situation where the internal implementation requires
        // the panel to be invisible, in which case we can set m_privateVisible=False
        // to make the panel keep invisible even if the user sets m_visible=True.
        // For example, to hide a menu item after it moves outside of its parent menu,
        // we can make it private-invisible without messing up the user settings.
        bool m_privateVisible = true;
        bool m_privateEnabled = true;

        D2D1_RECT_F m_rect = {};
        D2D1_RECT_F m_absoluteRect = {};

    public:
        virtual bool visible() const;
        virtual void setVisible(bool value);

        virtual bool enabled() const;
        virtual void setEnabled(bool value);

    protected:
        void setPrivateVisible(bool value);
        void setPrivateEnabled(bool value);

        void updateAppEventReactability();

    public:
        float width() const;
        float height() const;
        D2D1_SIZE_F size() const;

        // Returns size scaled by DPI.
        D2D1_SIZE_U pixelSize() const;

        D2D1_POINT_2F position() const;
        D2D1_POINT_2F absolutePosition() const;

        D2D1_RECT_F selfCoordRect() const;
        const D2D1_RECT_F& relativeRect() const;
        const D2D1_RECT_F& absoluteRect() const;

        D2D1_POINT_2F selfCoordToRelative(const D2D1_POINT_2F& p) const;
        D2D1_RECT_F selfCoordToRelative(const D2D1_RECT_F& rect) const;
        D2D1_POINT_2F selfCoordToAbsolute(const D2D1_POINT_2F& p) const;
        D2D1_RECT_F selfCoordToAbsolute(const D2D1_RECT_F& rect) const;

        D2D1_POINT_2F relativeToSelfCoord(const D2D1_POINT_2F& p) const;
        D2D1_RECT_F relativeToSelfCoord(const D2D1_RECT_F& rect) const;
        D2D1_POINT_2F relativeToAbsolute(const D2D1_POINT_2F& p) const;
        D2D1_RECT_F relativeToAbsolute(const D2D1_RECT_F& rect) const;

        D2D1_POINT_2F absoluteToSelfCoord(const D2D1_POINT_2F& p) const;
        D2D1_RECT_F absoluteToSelfCoord(const D2D1_RECT_F& rect) const;
        D2D1_POINT_2F absoluteToRelative(const D2D1_POINT_2F& p) const;
        D2D1_RECT_F absoluteToRelative(const D2D1_RECT_F& rect) const;

        // All transformation operations are based on the relative coordinate.

        void resize(const D2D1_SIZE_F& size);
        void resize(float width, float height);

        void move(const D2D1_POINT_2F& point);
        void move(float left, float top);

        void transform(const D2D1_RECT_F& rect);
        void transform(float left, float top, float width, float height);

    protected:
        void updateAbsoluteRect();

    protected:
        renderer::Renderer::CommandLayer::DrawObject2DSet m_drawObjects2D = {};

        WeakPtr<Panel> m_parent = {};

        using ChildObjectSet = ISortable<Panel>::ShrdPrioritySet;

        ChildObjectSet m_children = {};

        using ChildObjectTempSet = ISortable<Panel>::WeakPrioritySet;

        ChildObjectTempSet m_hitChildren = {};

        ChildObjectTempSet m_pinnedChildren = {}, m_diffPinnedChildren = {};

    public:
        virtual void registerDrawObjects();
        virtual void unregisterDrawObjects();

        void registerApplicationEvents();
        void unregisterApplicationEvents();

        void pinApplicationEvents();
        void unpinApplicationEvents();

        const WeakPtr<Panel>& parent() const;
        void setParent(ShrdPtrRefer<Panel> uiobj);

        const ChildObjectSet& children() const;

        void addUIObject(ShrdPtrRefer<Panel> uiobj);
        void removeUIObject(ShrdPtrRefer<Panel> uiobj);

        void pinUIObject(ShrdPtrRefer<Panel> uiobj);
        void unpinUIObject(ShrdPtrRefer<Panel> uiobj);

        void clearAddedUIObjects();
        void clearPinnedUIObjects();

    protected:
        void updateDiffPinnedUIObjects();
        void updateDiffPinnedUIObjectsLater();
    };

    template<typename T = Panel>
    SharedPtr<T> nullUIObj() { return nullptr; }

    template<typename T, typename... Types>
    SharedPtr<T> makeUIObject(Types&& ...args)
    {
        auto uiobj = std::make_shared<T>(args...);
        uiobj->onInitializeFinish();
        return uiobj;
    }

    template<typename T, typename... Types>
    SharedPtr<T> makeRootUIObject(Types&& ...args)
    {
        auto uiobj = makeUIObject<T>(args...);
        uiobj->registerDrawObjects();
        uiobj->registerApplicationEvents();
        return uiobj;
    }

    template<typename T, typename... Types>
    SharedPtr<T> makeManagedUIObject
    (ShrdPtrRefer<Panel> parent, Types&& ...args)
    {
        auto uiobj = makeUIObject<T>(args...);
        parent->addUIObject(uiobj);
        return uiobj;
    }
}
