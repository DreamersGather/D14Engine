#pragma once

#include "Common/Precompile.h"

// Do NOT remove this header for code tidy
// as the UI creation helper relies on it.
#include "Common/RuntimeError.h"

#include "Renderer/Interfaces/IDrawObject2D.h"

#include "UIKit/Application.h"
#include "UIKit/Event.h"

namespace d14engine::uikit
{
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

        ////////////////////////
        // UI Graphics (D2D1) //
        ////////////////////////

        //------------------------------------------------------------------
        // D2D1 Target
        //------------------------------------------------------------------
    public:
        using UICommandLayer = Application::UICommandLayer;
        using UIDrawTarget = Application::UIDrawTarget;

    protected:
        UIDrawTarget m_drawObjects = {};

    public:
        void registerDrawObject(ShrdPtrRefer<Panel> uiobj);
        void unregisterDrawObject(ShrdPtrRefer<Panel> uiobj);

        //------------------------------------------------------------------
        // UI Properties
        //------------------------------------------------------------------
    protected:
        bool m_visible = true;
        bool m_enabled = true;

    public:
        virtual bool visible() const;
        virtual void setVisible(bool value);

        virtual bool enabled() const;
        virtual void setEnabled(bool value);

        // Consider a situation where the internal implementation requires
        // the panel to keep invisible, and we can set m_privateVisible=False
        // to force it to be invisible even if the user sets m_visible=True.
    protected:
        bool m_privateVisible = true;
        bool m_privateEnabled = true;

        void setPrivateVisible(bool value);
        void setPrivateEnabled(bool value);

    protected:
        D2D1_RECT_F m_rect = {};
        D2D1_RECT_F m_absoluteRect = {};

        void updateAbsoluteRect();

    public:
        float width() const;
        float height() const;

        D2D1_SIZE_F size() const;
        // Returns size scaled by DPI.
        D2D1_SIZE_U pixelSize() const;

        void setSize(float width, float height);
        void setSize(const D2D1_SIZE_F& size);

        float x() const;
        float y() const;

        D2D1_POINT_2F position() const;

        float absoluteX() const;
        float absoluteY() const;

        D2D1_POINT_2F absolutePosition() const;

        void setPosition(float x, float y);
        void setPosition(const D2D1_POINT_2F& point);

        D2D1_RECT_F selfCoordRect() const;
        const D2D1_RECT_F& relativeRect() const;
        const D2D1_RECT_F& absoluteRect() const;

        void transform
        (
            float x, float y, float width, float height
        );
        void transform(const D2D1_RECT_F& rect);

    public:
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

        //------------------------------------------------------------------
        // Draw Properties
        //------------------------------------------------------------------
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
        bool m_isPlayAnimation = false;

    public:
        bool isPlayAnimation() const;

        void increaseAnimationCount();
        void decreaseAnimationCount();

    protected:
        using Renderer = renderer::Renderer;

        void updateChildrenObjects(Renderer* rndr);

        void drawChildrenLayers(Renderer* rndr);
        void drawBackground(Renderer* rndr);
        void drawChildrenObjects(Renderer* rndr);

    public:
        virtual void drawD2d1ObjectPreceding(Renderer* rndr);
        virtual void drawD2d1ObjectPosterior(Renderer* rndr);

        ////////////////////
        // UI Object Tree //
        ////////////////////

    protected:
        WeakPtr<Panel> m_parent = {};

        using ChildObjectSet = ISortable<Panel>::ShrdPrioritySet;

        ChildObjectSet m_children = {};

        using ChildObjectTempSet = ISortable<Panel>::WeakPrioritySet;

        ChildObjectTempSet m_hitChildren = {};

        ChildObjectTempSet m_pinnedChildren = {};

    public:
        const WeakPtr<Panel>& parent() const;
        void setParent(ShrdPtrRefer<Panel> uiobj);

        const ChildObjectSet& children() const;

        void registerUIEvents(ShrdPtrRefer<Panel> uiobj);
        void unregisterUIEvents(ShrdPtrRefer<Panel> uiobj);

        void addUIObject(ShrdPtrRefer<Panel> uiobj);
        void removeUIObject(ShrdPtrRefer<Panel> uiobj);

        void clearAddedUIObjects();

        void pinUIObject(ShrdPtrRefer<Panel> uiobj);
        void unpinUIObject(ShrdPtrRefer<Panel> uiobj);

        void clearPinnedUIObjects();

        //////////////////////
        // UI Priority Data //
        //////////////////////

    public:
        int drawObjectPriority() const;
        void setDrawObjectPriority(int value);

        int uiObjectPriority() const;
        void setUIObjectPriority(int value);

    protected:
        struct PriorityGroup
        {
            int drawObject = 0; // Set higher to draw above others.
            int uiObject = 0; // Set lower to get UI event first.
        };
        PriorityGroup m_frontPriorities = {};
        PriorityGroup m_backPriorities = {};

    public:
        const PriorityGroup& frontPriorities() const;
        const PriorityGroup& backPriorities() const;

        void bringToFront();
        void sendToBack();

        void bringChildObjectToFront(Panel* uiobj);
        void sendChildObjectToBack(Panel* uiobj);

        void reorderAbovePeerObject(Panel* uiobj);
        void reorderBelowPeerObject(Panel* uiobj);

        ///////////////////////
        // UI Event Callback //
        ///////////////////////

    public:
        template<bool presetBoolean>
        struct ApplicationEventFlag
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
        };

        // We want to find a word for describing whether the panel is able to
        // react or respond to the specific app-event, but unfortunately none.
        //
        // After consulting Limiao, we are determinded to use "reactability",
        // since we think it is very consistent with what we want to express.

        struct ApplicationEventReactability : ApplicationEventFlag<true>
        {
            bool hitTest = true;

            void set(bool value);
        }
        appEventReactability = {};

        struct ApplicationEventTransparency : ApplicationEventFlag<false>
        {
            // Here left blank intentionally.
        }
        appEventTransparency = {};

    protected:
        void updateAppEventReactability();

    public:
        //------------------------------------------------------------------
        // Life Cycle
        //------------------------------------------------------------------

        // Return whether the UI object is released successfully.

        bool release();

        Function<void(Panel*)> f_onRelease = {};

        bool releaseUIObject(ShrdPtrRefer<Panel> uiobj);

        Function<bool(Panel*, ShrdPtrRefer<Panel>)> f_onReleaseUIObject = {};

        //------------------------------------------------------------------
        // Basic
        //------------------------------------------------------------------

        bool isHit(const Event::Point& p) const;

        Function<bool(const Panel*, const Event::Point&)> f_isHit = {};

        void onGetMouseFocus();
        void onGetKeyboardFocus();

        Function<void(Panel*)> f_onGetMouseFocus = {};
        Function<void(Panel*)> f_onGetKeyboardFocus = {};

        void onLoseMouseFocus();
        void onLoseKeyboardFocus();

        Function<void(Panel*)> f_onLoseMouseFocus = {};
        Function<void(Panel*)> f_onLoseKeyboardFocus = {};

        bool holdMouseFocus() const;
        bool holdKeyboardFocus() const;

        //------------------------------------------------------------------
        // Size
        //------------------------------------------------------------------

        void onSize(SizeEvent& e);

        Function<void(Panel*, SizeEvent&)> f_onSize = {};

        void onParentSize(SizeEvent& e);

        Function<void(Panel*, SizeEvent&)> f_onParentSize = {};

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

        //------------------------------------------------------------------
        // Move
        //------------------------------------------------------------------

        void onMove(MoveEvent& e);

        Function<void(Panel*, MoveEvent&)> f_onMove = {};

        void onParentMove(MoveEvent& e);

        Function<void(Panel*, MoveEvent&)> f_onParentMove = {};

        //------------------------------------------------------------------
        // Mouse
        //------------------------------------------------------------------

        void onMouseEnter(MouseMoveEvent& e);

        Function<void(Panel*, MouseMoveEvent&)> f_onMouseEnter = {};

        void onMouseMove(MouseMoveEvent& e);

        Function<void(Panel*, MouseMoveEvent&)> f_onMouseMove = {};

        bool enableChildrenMouseMoveEvent = true;

        void onMouseLeave(MouseMoveEvent& e);

        Function<void(Panel*, MouseMoveEvent&)> f_onMouseLeave = {};

        bool forceSingleMouseEnterLeaveEvent = true;

        void onMouseButton(MouseButtonEvent& e);

        Function<void(Panel*, MouseButtonEvent&)> f_onMouseButton = {};

        void onMouseWheel(MouseWheelEvent& e);

        Function<void(Panel*, MouseWheelEvent&)> f_onMouseWheel = {};

        //------------------------------------------------------------------
        // Keyboard
        //------------------------------------------------------------------

        void onKeyboard(KeyboardEvent& e);

        Function<void(Panel*, KeyboardEvent&)> f_onKeyboard = {};

        //------------------------------------------------------------------
        // Miscellaneous
        //------------------------------------------------------------------

        using ThemeStyle = Application::ThemeStyle;

        void onChangeThemeStyle(const ThemeStyle& style);

        Function<void(Panel*, const ThemeStyle&)> f_onChangeThemeStyle = {};

        bool enableChangeThemeStyleUpdate = true;

        void onChangeLangLocale(WstrRefer codeName);

        Function<void(Panel*, WstrRefer)> f_onChangeLangLocale = {};

        bool enableChangeLangLocaleUpdate = true;

        //------------------------------------------------------------------
        // D2D1 Object
        //------------------------------------------------------------------

        bool isD2d1ObjectVisible() const override;

        void setD2d1ObjectVisible(bool value) override;

        void onRendererUpdateObject2D(Renderer* rndr) override;

        Function<void(Panel*, Renderer*)>
            f_onRendererUpdateObject2DBefore = {},
            f_onRendererUpdateObject2DAfter = {};

        void onRendererDrawD2d1Layer(Renderer* rndr) override;

        Function<void(Panel*, Renderer*)>
            f_onRendererDrawD2d1LayerBefore = {},
            f_onRendererDrawD2d1LayerAfter = {};

        void onRendererDrawD2d1Object(Renderer* rndr) override;

        Function<void(Panel*, Renderer*)>
            f_onRendererDrawD2d1ObjectBefore = {},
            f_onRendererDrawD2d1ObjectAfter = {};

    protected:
        // Introduce func-helper to solve the inheritance conflicts of
        // the "override", "before" and "after" event callback lambdas.
        //
        // *-------*----------------------------------*--------------------------*
        // | Class | func                             | funcHelper               |
        // *-------*----------------------------------*--------------------------*
        // | A     | "before"; A::funcHelper; "after" | A's works                |
        // *-------*----------------------------------*--------------------------*
        // | B : A | "before"; B::funcHelper; "after" | A::funcHelper; B's works |
        // *-------*----------------------------------*--------------------------*
        //
        // To sum up, do the actual works in func-helper and call it in func.

        //------------------------------------------------------------------
        // Life Cycle
        //------------------------------------------------------------------

        virtual bool releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj);

        //------------------------------------------------------------------
        // Basic
        //------------------------------------------------------------------

        virtual bool isHitHelper(const Event::Point& p) const;

        virtual void onGetMouseFocusHelper();
        virtual void onGetKeyboardFocusHelper();

        virtual void onLoseMouseFocusHelper();
        virtual void onLoseKeyboardFocusHelper();

        //------------------------------------------------------------------
        // Size
        //------------------------------------------------------------------

        virtual void onSizeHelper(SizeEvent& e);
        virtual void onParentSizeHelper(SizeEvent& e);

        //------------------------------------------------------------------
        // Move
        //------------------------------------------------------------------

        virtual void onMoveHelper(MoveEvent& e);
        virtual void onParentMoveHelper(MoveEvent& e);

        //------------------------------------------------------------------
        // Mouse
        //------------------------------------------------------------------

        virtual void onMouseEnterHelper(MouseMoveEvent& e);
        virtual void onMouseMoveHelper(MouseMoveEvent& e);
        virtual void onMouseLeaveHelper(MouseMoveEvent& e);

        virtual void onMouseButtonHelper(MouseButtonEvent& e);
        virtual void onMouseWheelHelper(MouseWheelEvent& e);

        //------------------------------------------------------------------
        // Keyboard
        //------------------------------------------------------------------

        virtual void onKeyboardHelper(KeyboardEvent& e);

        //------------------------------------------------------------------
        // Miscellaneous
        //------------------------------------------------------------------

        virtual void onChangeThemeStyleHelper(const ThemeStyle& style);
        virtual void onChangeLangLocaleHelper(WstrRefer codeName);

        //------------------------------------------------------------------
        // D2D1 Object
        //------------------------------------------------------------------

        virtual void onRendererUpdateObject2DHelper(Renderer* rndr);
        virtual void onRendererDrawD2d1LayerHelper(Renderer* rndr);
        virtual void onRendererDrawD2d1ObjectHelper(Renderer* rndr);
    };

    /////////////////////////
    // UI Creation Helpers //
    /////////////////////////

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
        THROW_IF_NULL(Application::g_app);
        auto uiobj = makeUIObject<T>(args...);
        Application::g_app->addUIObject(uiobj);
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
