#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/Window.h"
#include "UIKit/DraggablePanel.h"
#include "UIKit/ResizablePanel.h"
#include "UIKit/ShadowMask.h"

namespace d14engine::uikit
{
    struct IconLabel;
    struct TabGroup;

    struct Window : appearance::Window, DraggablePanel, ResizablePanel
    {
        Window(
            ShrdPtrRefer<IconLabel> caption,
            const D2D1_RECT_F& rect = {},
            float captionPanelHeight = 32.0f,
            float decorativeBarHeight = 4.0f);

        Window(
            WstrRefer title = L"Untitled",
            const D2D1_RECT_F& rect = {},
            float captionPanelHeight = 32.0f,
            float decorativeBarHeight = 4.0f);

        void onInitializeFinish() override;

        _D14_SET_APPEARANCE_PROPERTY(Window)

        //////////////////////
        // Cached Resources //
        //////////////////////

        using MasterPtr = cpp_lang_utils::EnableMasterPtr<Window>;

        struct DrawBufferRes : MasterPtr
        {
            using MasterPtr::MasterPtr;

            ShadowMask shadowMask = {};
            void loadShadowMask();

            MaskObject& mask();
            ShadowMask& shadow();

            ComPtr<ID2D1BitmapBrush1> brush = {};
            void loadBrush();
        }
        drawBufferRes{ this };

        struct DecorativeBarRes : MasterPtr
        {
            using MasterPtr::MasterPtr;

            ComPtr<ID2D1LinearGradientBrush> brush = {};
            void loadBrush();
        }
        decorativeBarRes{ this };

        ////////////////////////
        // Callback Functions //
        ////////////////////////

        //------------------------------------------------------------------
        // Public Interfaces
        //------------------------------------------------------------------
    public:
        void onMinimize();

        Function<void(Window*)> f_onMinimize = {};

        void onMaximize();

        Function<void(Window*)> f_onMaximize = {};

        void onRestore();

        Function<void(Window*)> f_onRestore = {};

        void onClose();

        Function<void(Window*)> f_onClose = {};

        //------------------------------------------------------------------
        // Protected Helpers
        //------------------------------------------------------------------
    protected:
        virtual void onMinimizeHelper();
        virtual void onMaximizeHelper();
        virtual void onRestoreHelper();
        virtual void onCloseHelper();

        /////////////////////////
        // Graphics Components //
        /////////////////////////

        //------------------------------------------------------------------
        // Children Objects
        //------------------------------------------------------------------
    protected:
        SharedPtr<IconLabel> m_caption = {};

        SharedPtr<Panel> m_content = {};

    public:
        const SharedPtr<IconLabel>& caption() const;
        void setCaption(ShrdPtrRefer<IconLabel> caption);

        template<typename T = Panel>
        WeakPtr<T> content() const
        {
            if constexpr (std::is_same_v<T, Panel>) return m_content;
            else return std::dynamic_pointer_cast<T>(m_content);
        }
        void setContent(ShrdPtrRefer<Panel> uiobj);

        //------------------------------------------------------------------
        // Drawing Properties
        //------------------------------------------------------------------
    protected:
        float m_captionPanelHeight = {};
        float m_decorativeBarHeight = {};

        D2D1_RECT_F captionPanelAbsoluteRect() const;
        D2D1_RECT_F decorativeBarAbsoluteRect() const;

        D2D1_RECT_F captionTitleSelfcoordRect() const;

    public:
        float captionPanelHeight() const;
        void setCaptionPanelHeight(float value);

        float decorativeBarHeight() const;
        void setDecorativeBarHeight(float value);

        float clientAreaHeight() const;
        D2D1_RECT_F clientAreaSelfcoordRect() const;

        constexpr static float nonClientAreaMinimalWidth()
        {
            return 144.0f;
        }
        constexpr static float nonClientAreaDefaultHeight()
        {
            return 36.0f;
        }
        float nonClientAreaHeight() const;
        D2D1_RECT_F nonClientAreaSelfcoordRect() const;
        D2D1_RECT_F nonClientAreaMinimalSelfcoordRect() const;

    protected:
        // Button panel consists of 3 buttons, from left to right:
        // (Minimize Button) (Maximize/Restore Button) (Close Button)

        constexpr static float button1Width()
        {
            return 32.0f;
        }
        constexpr static float button2Width()
        {
            return 32.0f;
        }
        constexpr static float button3Width()
        {
            return 40.0f;
        }
        constexpr static float buttonHeight()
        {
            return 24.0f;
        }
        constexpr static float buttonPanelWidth()
        {
            return button1Width() + button2Width() + button3Width();
        }
        constexpr static float buttonPanelLeftmostOffset()
        {
            return buttonPanelRightmostOffset() + buttonPanelWidth();
        }
        constexpr static float buttonPanelRightmostOffset()
        {
            return 20.0f;
        }
        D2D1_RECT_F button1AbsoluteRect() const;
        D2D1_RECT_F button2AbsoluteRect() const;
        D2D1_RECT_F button3AbsoluteRect() const;

        constexpr static D2D1_RECT_F minimizeIconVertexOffset()
        {
            return { 10.0f, 14.0f, -10.0f, -8.0f };
        }
        constexpr static D2D1_RECT_F maximizeIconVertexOffset()
        {
            return { 11.0f, 7.0f, -11.0f, -7.0f };
        }
        constexpr static float maximizeIconStrokeWidth()
        {
            return 2.0f;
        }
        constexpr static D2D1_RECT_F restoreIconVertexOffset()
        {
            return { 11.0f, 10.0f, -14.0f, -7.0f };
        }
        constexpr static float restoreOrnamentOffset()
        {
            return 3.0f;
        }
        constexpr static float restoreIconStrokeWidth()
        {
            return 2.0f;
        }
        constexpr static D2D1_RECT_F closeIconVertexOffset()
        {
            return { 15.0f, 7.0f, -15.0f, -7.0f };
        }
        constexpr static float closeIconStrokeWidth()
        {
            return 2.0f;
        }
        D2D1_RECT_F minimizeIconAbsoluteRect() const;
        D2D1_RECT_F minimizeButtonAbsoluteRect() const;

        D2D1_RECT_F maximizeIconAbsoluteRect() const;
        D2D1_RECT_F maximizeButtonAbsoluteRect() const;

        D2D1_RECT_F restoreIconAbsoluteRect() const;
        D2D1_RECT_F restoreButtonAbsoluteRect() const;

        D2D1_RECT_F closeIconAbsoluteRect() const;
        D2D1_RECT_F closeButtonAbsoluteRect() const;

        ///////////////////////
        // Interaction Logic //
        ///////////////////////

        //------------------------------------------------------------------
        // Display State
        //------------------------------------------------------------------
    public:
        enum class DisplayState { Normal, Minimized, Maximized };

        constexpr static auto Normal = DisplayState::Normal;
        constexpr static auto Minimized = DisplayState::Minimized;
        constexpr static auto Maximized = DisplayState::Maximized;

    protected:
        DisplayState m_displayState = Normal;

    public:
        DisplayState displayState() const;
        void setDisplayState(DisplayState state);

        //------------------------------------------------------------------
        // Button Panel
        //------------------------------------------------------------------
    public:
        bool button1Enabled = true, button2Enabled = true, button3Enabled = true;

        bool& minimizeButtonEnabled = button1Enabled;
        bool& maximizeButtonEnabled = button2Enabled;
        bool& restoreButtonEnabled = button2Enabled;
        bool& closeButtonEnabled = button3Enabled;

    protected:
        // Indicates whether a special operation is being performed on the
        // window (e.g. resizing, dragging). When this is true, the buttons
        // should not respond UI events even if they are enabled indeed.
        bool isPerformSpecialOperation() const;

        bool m_isButton1Hover = false, m_isButton1Down = false;
        bool m_isButton2Hover = false, m_isButton2Down = false;
        bool m_isButton3Hover = false, m_isButton3Down = false;

        ButtonState getButton1State(bool isHover, bool isDown) const;
        ButtonState getButton2State(bool isHover, bool isDown) const;
        ButtonState getButton3State(bool isHover, bool isDown) const;

        void setIconBrushState(ButtonState state);
        void setButtonBrushState(ButtonState state);

        //------------------------------------------------------------------
        // TabGroup Linkage
        //------------------------------------------------------------------
    protected:
        using TabGroupSet = std::set<WeakPtr<TabGroup>, std::owner_less<WeakPtr<TabGroup>>>;

        // Only the registered tab-groups can be associated with the window.
        TabGroupSet m_registeredTabGroups = {};

    public:
        void registerTabGroup(WeakPtrRefer<TabGroup> tg);
        void unregisterTabGroup(WeakPtrRefer<TabGroup> tg);

    public:
        Function<void(Window*, TabGroup*)> f_onTriggerTabDemoting = {};

        // When a window is being dragged, all of the tab-groups that have
        // been registered for the window will be associated with it, and if
        // the window is then released above any associated tab-group, its
        // caption and content will be demoted and inserted as a new tab and
        // the original window will be destroyed later.

        WeakPtr<TabGroup> associatedTabGroup = {};

    protected:
        void handleMouseMoveForRegisteredTabGroups(MouseMoveEvent& e);
        void handleMouseButtonForRegisteredTabGroups(MouseButtonEvent& e);

        //------------------------------------------------------------------
        // Miscellaneous
        //------------------------------------------------------------------
    public:
        // Whether to move to the topmost when receiving mouse-button event.
        bool respondSetForegroundEvent = true;

        /////////////////////////
        // Interface Overrides //
        /////////////////////////

    public:
        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        float minimalWidth() const override;

        float minimalHeight() const override;

    protected:
        //------------------------------------------------------------------
        // IDrawObject2D
        //------------------------------------------------------------------

        void onRendererUpdateObject2DHelper(Renderer* rndr) override;

        void onRendererDrawD2d1LayerHelper(Renderer* rndr) override;

        void onRendererDrawD2d1ObjectHelper(Renderer* rndr) override;

        void drawD2d1ObjectPosterior(Renderer* rndr) override;

        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        bool releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj) override;

        void onSizeHelper(SizeEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        void onMouseMoveHelper(MouseMoveEvent& e) override;

        void onMouseLeaveHelper(MouseMoveEvent& e) override;

        void onMouseButtonHelper(MouseButtonEvent& e) override;

        //------------------------------------------------------------------
        // DraggablePanel
        //------------------------------------------------------------------

        bool isTriggerDraggingHelper(const Event::Point& p) override;
    };
}
