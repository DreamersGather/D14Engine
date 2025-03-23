#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/EnumMagic.h"

#include "Renderer/Renderer.h"

#include "UIKit/Appearances/Appearance.h"

namespace d14engine::uikit
{
    struct Cursor;
    struct Panel;
    struct TextInputObject;

    struct Application : cpp_lang_utils::NonCopyable
    {
        friend Cursor;
        friend Panel;

        struct CreateInfo
        {
            using LibraryPathArray = std::vector<Wstring>;
#ifdef _WIN64
            LibraryPathArray libraryPaths =
            {
                L"Lib/DirectXShaderCompiler/x64"
            };
#else
            LibraryPathArray libraryPaths =
            {
                L"Lib/DirectXShaderCompiler/x86"
            };
#endif
            // Win32 window's class name & window name (i.e. caption text)
            Wstring name = L"D14Engine";

            // Resources (such as cursors) are loaded from this path.
            Wstring binaryPath = L"Bin/";

            Wstring cursorPath() const
            {
                return binaryPath + L"Cursors/";
            }
            Optional<float> dpi = {};

            // In Device-Independent-Pixel
            SIZE windowSize = { 800, 600 };

            bool fullscreen = false;

            bool excludeFromCapture = false;

            // Whether to enable the DXGI duplication.
            bool duplication = false;

            // Whether to enable the WDDM composition.
            bool composition = false;
        };

        static Application* g_app;

        explicit Application(const CreateInfo& info = {});

        virtual ~Application();

        // This field stores the original create-info passed in the ctor.
        const CreateInfo createInfo = {};

        ////////////////////
        // Initialization //
        ////////////////////

    private:
        void initWin32Window();

        void initDirectX12Renderer();

        void initMiscComponents();

    public:
        int run(FuncRefer<void(Application* app)> onLaunch = {});

        void exit(); // The exit code is returned by calling run.

        //////////////////
        // Win32 Window //
        //////////////////

    private:
        HWND m_win32Window = {};

    public:
        HWND win32Window() const;

        //------------------------------------------------------------------
        // Window Process
        //------------------------------------------------------------------
    private:
        static LRESULT CALLBACK fnWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        //------------------------------------------------------------------
        // Window Settings
        //------------------------------------------------------------------
    public:
        struct Win32WindowSettings
        {
            struct Callback
            {
                Function<void()> f_onRestoreFromMinimized = {};
                Function<void(const SIZE&)> f_onClientAreaSize = {};
            }
            callback = {};

            struct Geometry
            {
                Optional<POINT> minTrackSize = {};
            }
            geometry = {};

            struct SizingFrame
            {
                Optional<LONG> frameWidth = {};
                Function<LRESULT(const POINT&)> f_NCHITTEST = {};
            }
            sizingFrame = {};
        }
        win32WindowSettings = {};

        //------------------------------------------------------------------
        // NCHITTEST Handler
        //------------------------------------------------------------------
    private:
        LRESULT handleWin32NCHITTESTMessage(const POINT& pt);

    public:
        LRESULT defWin32NCHITTESTMessageHandler(const POINT& pt);

        ///////////////////
        // DX12 Renderer //
        ///////////////////

    private:
        using Renderer = renderer::Renderer;

        UniquePtr<Renderer> m_renderer = {};

    public:
        Renderer* renderer() const;

        //------------------------------------------------------------------
        // UI Command Layer
        //------------------------------------------------------------------
    public:
        using UICommandLayer = Renderer::CommandLayer;
        using UIDrawTarget = UICommandLayer::D2D1Target;

    private:
        SharedPtr<UICommandLayer> m_uiCmdLayer = {};

        UIDrawTarget& drawObjects(); // D2D1Target of m_uiCmdLayer

    public:
        constexpr static int g_uiCmdLayerPriority = 200;

        const SharedPtr<UICommandLayer>& uiCmdLayer() const;

        void registerDrawObject(ShrdPtrRefer<Panel> uiobj);
        void unregisterDrawObject(ShrdPtrRefer<Panel> uiobj);

        //------------------------------------------------------------------
        // Resource Capture
        //------------------------------------------------------------------
    public:
        ComPtr<ID2D1Bitmap1> windowshot() const;

        // This is valid only when the DXGI duplication is enabled.
        Optional<ComPtr<ID2D1Bitmap1>> screenshot() const;

        //------------------------------------------------------------------
        // Animation Control
        //------------------------------------------------------------------
    private:
        // Indicates how many UI objects are playing animations.
        UINT m_animationCount = 0;

    public:
        UINT animationCount() const;

        void increaseAnimationCount();
        void decreaseAnimationCount();

        ////////////////////
        // UI Object Tree //
        ////////////////////

    private:
        using UIObjectSet = ISortable<Panel>::ShrdPrioritySet;

        UIObjectSet m_uiObjects = {};

        using UIObjectTempSet = ISortable<Panel>::WeakPrioritySet;

        UIObjectTempSet m_hitUIObjects = {};

        // The pinned UI objects keep receiving all UI events even not hit.
        // Tips: This can be used to implement a global hotspot mechanism,
        // such as a shortcut key manager or a background auxiliary object.

        UIObjectTempSet m_pinnedUIObjects = {};

    public:
        const UIObjectSet& uiObjects() const;

        void registerUIEvents(ShrdPtrRefer<Panel> uiobj);
        void unregisterUIEvents(ShrdPtrRefer<Panel> uiobj);

        void addUIObject(ShrdPtrRefer<Panel> uiobj);
        void removeUIObject(ShrdPtrRefer<Panel> uiobj);

        void clearAddedUIObjects();

        void pinUIObject(ShrdPtrRefer<Panel> uiobj);
        void unpinUIObject(ShrdPtrRefer<Panel> uiobj);

        void clearPinnedUIObjects();

    public:
        //------------------------------------------------------------------
        // A focused UI object will exclusively handle all related events:
        //------------------------------------------------------------------
        // 1) Mouse:
        //    a) Dragging or resizing a panel
        //    b) Dragging a scrollbar to move content
        //    c) Dragging a slider to adjust value
        //
        //------------------------------------------------------------------
        // 2) Keyboard:
        //    a) Typing text in a text-input object
        //    b) Navigating through a waterfall view
        //
        //------------------------------------------------------------------

        enum class FocusType { Mouse, Keyboard };

    private:
        using FocusedTempMap = cpp_lang_utils::EnumMap<FocusType, WeakPtr<Panel>>;

        FocusedTempMap m_focusedUIObjects = {};

    public:
        void focusUIObject(FocusType focus, ShrdPtrRefer<Panel> uiobj);

    private:
        // The UI objects are maintained with a std::set, and the UI event
        // callbacks are performed in order within each Win32 wnd-proc.
        //
        // Under normal circumstances, the callbacks are invoked according to
        // the receiving order of the Win32 messages, in which case there is
        // no conflict between each callback. If a callback, however, sends
        // an immediate Win32 message while being handled, the Win32 wnd-proc
        // will retrieve the std::set stores the UI objects again and try to
        // modify it to respond to the new UI event, and this may cause some
        // undefined results because we are trying to insert/erase a std::set
        // while traversing it with the corresponding volatie iterator.
        // (PS: That operation is invalid for all STL associated containers).
        //
        // To solve the problem, we are determined to introduce a flag about
        // whether the Win32 wnd-proc is handling any UI event, and the newly
        // received Win32 messages will be added to the system message queue.

        bool m_isHandlingSensitiveUIEvent = false;

    public:
        // Set this to True to notify the window process to post a message
        // for immediate mouse-move after processing all mouse-move events.
        bool sendNextImmediateMouseMoveEvent = false;

    private:
        // If sendNextImmediateMouseMoveEvent is set in the previous update,
        // this helps post a new message of immediate mouse-move event,
        // which is useful for keeping the related UI objects up to date.
        void handleImmediateMouseMoveEventCallback();

    public:
        // Affects the max number of mouse-enter/leave events in each update.
        // During each event handling of the message loop, the window-process:
        // (True) only checks the highest-priority UI object.
        // (False) traverses and checks all candidate UI objects.
        bool forceSingleMouseEnterLeaveEvent = true;

    private:
        WeakPtr<TextInputObject> getFocusedTextInputObject() const;

        ///////////////////
        // Priority Data //
        ///////////////////

    private:
        // When adding new UI objects, these priority attributes are updated.
        // When we need to move a specific UI object to the front or back,
        // these priority attributes can be refered for the related settings.

        struct PriorityGroup
        {
            int drawObject = 0; // Set higher to draw above others.
            int uiObject = 0; // Set lower to get UI event first.
        };
        PriorityGroup m_frontPriorities = {};
        PriorityGroup m_backPriorities = {};

    public:
        // It is worth noting that these priority attributes does not accurately
        // represent the current priority status of the UI object collection;
        // in other words, the priority of the topmost UI object is not necessarily
        // equal to frontPriority. To obtain the priority of the topmost UI object,
        // directly use (*uiObjects().begin/rbegin())->d2d1/uiObjectPriority().

        const PriorityGroup& frontPriorities() const;
        const PriorityGroup& backPriorities() const;

        void bringRootObjectToFront(Panel* uiobj);
        void sendRootObjectToBack(Panel* uiobj);

        ///////////////////////
        // Global UI Objects //
        ///////////////////////

        //------------------------------------------------------------------
        // Cursor
        //------------------------------------------------------------------
    private:
        SharedPtr<Cursor> m_cursor = {};

        D2D1_POINT_2F m_lastCursorPoint = {};

    public:
        Cursor* cursor() const;

        const D2D1_POINT_2F& lastCursorPoint() const;

        // Note that the access level is public, so any object can modify this.
        // Therefore, this is merely a flag indicating that a dragging might occur.
        // If this is set, the window process may skip updating cursor's position,
        // as the cursor should be fixed relative to the window during a dragging.
        // Generally, only DraggablePanel and its inheritors may modify this flag.
        bool isTriggerDraggingWin32Window = false;

        ///////////////////
        // Miscellaneous //
        ///////////////////

        //------------------------------------------------------------------
        // Theme Style
        //------------------------------------------------------------------
    public:
        using ThemeStyle = appearance::Appearance::ThemeStyle;

    private:
        static Wstring querySystemThemeName();
        static D2D1_COLOR_F querySystemThemeColor();

    public:
        static ThemeStyle querySystemThemeStyle();

    private:
        ThemeStyle m_themeStyle = {};

    public:
        const ThemeStyle& themeStyle() const;
        void setThemeStyle(const ThemeStyle& style);

        Function<void(const ThemeStyle&)> f_onSystemThemeStyleChange = {};

        //------------------------------------------------------------------
        // Language & Locale
        //------------------------------------------------------------------
    private:
        // A lang-locale code from ISO-693 list (e.g. "en-us" and "zh-cn" etc.).
        Wstring m_langLocale = L"en-us";

    public:
        WstrRefer langLocale() const;
        void setLangLocale(WstrRefer codeName);

        ///////////////////////
        // Custom Extensions //
        ///////////////////////

        //------------------------------------------------------------------
        // Custom Messages
        //------------------------------------------------------------------
    public:
        enum class CustomMessage
        {
            // Messages below WM_USER are reserved.
            _UserDefinedStart_ = WM_USER,

            // When there is no regular user input event,
            // this can be used to activate the message loop.
            // For example, when switching to animation mode,
            // this ensures running the update-draw loop.
            AwakeGetMessage,

            // Used to implement non-blocking callbacks
            // for events triggered within other threads.
            HandleThreadEvent
        };
        void postCustomMessage(CustomMessage message, WPARAM wParam = 0, LPARAM lParam = 0);

        //------------------------------------------------------------------
        // Multi-threading
        //------------------------------------------------------------------
        // Calling triggerThreadEvent in a child thread will
        // trigger the callback set by registerThreadCallback
        // in the main UI thread (associated via the ThreadEventID).
        //------------------------------------------------------------------
        // auto app = Application::g_app;
        // Application::ThreadID id = 14;
        //
        // button->f_onMouseButton = [=](auto p, auto e)
        // {
        //     std::thread([=]
        //     {
        //         int sum = 1 + 1;
        //         app->triggerThreadEvent(id, sum);
        //     })
        //     .detach();
        // };
        // app->registerThreadCallback(id, [=](auto data)
        // {
        //     assert(data == 2);
        // }
        //------------------------------------------------------------------
    public:
#ifdef _WIN64
        using ThreadEventID = UINT64; // matches WPARAM
        using ThreadEventData = UINT64; // matches LPARAM
#else
        using ThreadEventID = UINT32; // matches WPARAM
        using ThreadEventData = UINT32; // matches LPARAM
#endif
        using ThreadCallback = Function<void(ThreadEventData)>;
        using ThreadCallbackParam = const ThreadCallback&;

    private:
        using ThreadCallbackMap = std::unordered_map<ThreadEventID, ThreadCallback>;

        ThreadCallbackMap m_threadCallbacks = {};

    public:
        void registerThreadCallback(ThreadEventID id, ThreadCallbackParam callback);
        void unregisterThreadCallback(ThreadEventID id);

        void triggerThreadEvent(ThreadEventID id, ThreadEventData data = {});
    };
}
