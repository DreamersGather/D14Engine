#pragma once

#include "Common/Precompile.h"

#include "Renderer/Renderer.h"

#include "UIKit/Appearances/Appearance.h"

namespace d14engine::uikit
{
    struct Cursor;
    struct Panel;
    struct TextInputObject;

    struct Application : cpp_lang_utils::NonCopyable
    {
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

        // After exiting the program, the application is destroyed at first,
        // and other objects are destroyed subsequently, in which situation
        // the program will crash if their dtors use the global application
        // pointer (i.e. g_app) to clear the maintained resources.
        //
        // Set g_app to nullptr after the application destroyed to help the
        // objects decide whether there's need to do the clearing.

        virtual ~Application() { g_app = nullptr; }

        // This field stores the original create-info passed in the ctor.
        const CreateInfo createInfo = {};

    private:
        void initWin32Window();

        void initDirectX12Renderer();

        void initMiscComponents();

    public:
        int run(FuncRefer<void(Application* app)> onLaunch = {});

        void exit(); // The exit code is returned by calling run.

        /////////////////////////////////
        // Win32 Window & Message Loop //
        /////////////////////////////////

    private:
        static LRESULT CALLBACK fnWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        HWND m_win32Window = {};

        LRESULT handleWin32NCHITTESTMessage(const POINT& pt);

    public:
        HWND win32Window() const;

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

        LRESULT defWin32NCHITTESTMessageHandler(const POINT& pt);

        ///////////////////////////////
        // Renderer & UI Object Tree //
        ///////////////////////////////

    private:
        UniquePtr<renderer::Renderer> m_renderer = {};

    public:
        renderer::Renderer* dx12Renderer() const;

    public:
        // This is valid only when the DXGI duplication is enabled.
        Optional<ComPtr<ID2D1Bitmap1>> screenshot() const;

        ComPtr<ID2D1Bitmap1> windowshot() const;

    private:
        // Indicates how many UI objects are playing animations.
        UINT m_animationCount = 0;

    public:
        UINT animationCount() const;

        void increaseAnimationCount();
        void decreaseAnimationCount();

    private:
        struct TopmostPriority
        {
            int d2d1Object = INT_MIN;
            int uiObject = INT_MAX;
        }
        m_topmostPriority = {};

    public:
        const TopmostPriority& topmostPriority() const;

        void moveRootObjectTopmost(Panel* uiobj);

    private:
        SharedPtr<renderer::Renderer::CommandLayer> m_uiCmdLayer = {};

        using UIObjectSet = ISortable<Panel>::ShrdPrioritySet;

        UIObjectSet m_uiObjects = {};

        using UIObjectTempSet = ISortable<Panel>::WeakPrioritySet;

        UIObjectTempSet m_hitUIObjects = {};

        // The pinned UI objects keep receiving UI events while not hitting.
        //
        // Introduce the diff-pinned UI objects to avoid the hit UI objects
        // receiving the same event repeatedly (Diff-pinned = Hit - Pinned).

        UIObjectTempSet m_pinnedUIObjects = {}, m_diffPinnedUIObjects = {};

    public:
        constexpr static int g_uiCmdLayerPriority = 200;

        const SharedPtr<renderer::Renderer::CommandLayer>& uiCmdLayer() const;

        const UIObjectSet& uiObjects() const;

        void addUIObject(ShrdPtrRefer<Panel> uiobj);
        void removeUIObject(ShrdPtrRefer<Panel> uiobj);

        void pinUIObject(ShrdPtrRefer<Panel> uiobj);
        void unpinUIObject(ShrdPtrRefer<Panel> uiobj);

        void clearAddedUIObjects();
        void clearPinnedUIObjects();

        // Affects the maximum number of mouse-enter/leave events.
        // In each event handling of message loop, the window process:
        // (True) only checks the highest-priority UI object.
        // (False) traverses and checks all candidate UI objects.
        bool forceSingleMouseEnterLeaveEvent = true;

    private:
        void updateDiffPinnedUIObjects();

        // Updating the diff-pinned UI objects while handling UI events may cause
        // undefined behavior (e.g. modifying a std container while iterating it).
        // To solve this problem, we are determined to send a custom Win32 message
        // that notifies the actual updating during next Win32 message delivering.
        void updateDiffPinnedUIObjectsLater();

    private:
        struct FocusedUIObject
        {
            WeakPtr<Panel> mouse = {};
            WeakPtr<Panel> keyboard = {};
        }
        m_focusedUIObject = {};

    public:


    private:
        WeakPtr<Panel> m_currFocusedUIObject = {};

    public:
        WeakPtr<Panel> currFocusedUIObject() const;
        void focusUIObject(ShrdPtrRefer<Panel> uiobj);

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

    private:
        Wstring m_langLocale = L"en-us";

    public:
        const Wstring& langLocale() const;
        void setLangLocale(WstrRefer codeName);

    private:
        WeakPtr<TextInputObject> m_focusedTextInputObject = {};

        void broadcastInputStringEvent(WstrRefer content);

    public:
        bool skipHandleNextFocusChangeEvent = false;

        // Set this to True to notify the window process to
        // post a new message of immediate mouse-move event.
        bool sendNextImmediateMouseMoveEvent = false;

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

        // If sendNextImmediateMouseMoveEvent is set in the previous update,
        // this helps post a new message of immediate mouse-move event,
        // which is useful for keeping the related UI objects up to date.
        void handleImmediateMouseMoveEventCallback();

        //////////////////////////////
        // Additional Functionality //
        //////////////////////////////

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

            // Used to help update diff-pinned UI object set.
            UpdateRootDiffPinnedUIObjects,
            UpdateMiscDiffPinnedUIObjects,

            // Used to implement non-blocking callbacks
            // for events triggered within other threads.
            HandleThreadEvent
        };
        void postCustomMessage(CustomMessage message, WPARAM wParam = 0, LPARAM lParam = 0);

        //---------------------------------------------------------
        // Diff-Pinned UI Objects
        //---------------------------------------------------------
        // Similar to updateDiffPinnedUIObjectsLater,
        // we also need an "indirection" for non-root UI objects.
        //---------------------------------------------------------

    private:
        using DiffPinnedUpdateCandidateSet =
            std::set<WeakPtr<Panel>, std::owner_less<WeakPtr<Panel>>>;

        DiffPinnedUpdateCandidateSet m_diffPinnedUpdateCandidates = {};

    public:
        void pushDiffPinnedUpdateCandidate(ShrdPtrRefer<Panel> uiobj);

        //---------------------------------------------------------
        // Multi-threading
        //---------------------------------------------------------
        // Calling triggerThreadEvent in a child thread will
        // trigger the callback set by registerThreadCallback
        // in the main UI thread (associated via the ThreadEventID).
        //---------------------------------------------------------
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
        //---------------------------------------------------------

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
