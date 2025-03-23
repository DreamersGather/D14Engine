#include "Common/Precompile.h"

#include "UIKit/Application.h"

#include "Common/CppLangUtils/PointerCompare.h"
#include "Common/DirectXError.h"
#include "Common/MathUtils/GDI.h"

#include "Renderer/TickTimer.h"

#include "UIKit/BitmapUtils.h"
#include "UIKit/ColorUtils.h"
#include "UIKit/Cursor.h"
#include "UIKit/PlatformUtils.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/TextInputObject.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    Application* Application::g_app = nullptr;

    Application::Application(const CreateInfo& info) : createInfo(info)
    {
        g_app = this;

        ///////////////////
        // DPI Awareness //
        ///////////////////

        // Place this setting at the very beginning to ensure that the MessageBox
        // with relevant information of initialization errors also supports HiDPI.
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        //////////////////
        // Library Path //
        //////////////////

        Wstring exePath(MAX_PATH, 0);
        GetModuleFileName(nullptr, exePath.data(), MAX_PATH);
        // There is no need to check the result of find_last_of
        // because the exe-path is guaranteed to contain a "\".
        exePath.resize(exePath.find_last_of(L'\\') + 1);

        // Set this before calling AddDllDirectory to ensure that
        // subsequent LoadLibrary calls search user-defined paths.
        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

        for (auto& libPath : info.libraryPaths)
        {
            // Special note: AddDllDirectory only accepts absolute paths!
            THROW_IF_NULL(AddDllDirectory((exePath + libPath).c_str()));
        }
        ////////////////////
        // Initialization //
        ////////////////////

        initWin32Window();

        initDirectX12Renderer();

        initMiscComponents();
    }

    Application::~Application()
    {
        // After exiting the program, the application is destroyed at first,
        // and other objects are destroyed subsequently, in which situation
        // the program will crash if their dtors use the global application
        // pointer (i.e. g_app) to clear the maintained resources.
        //
        // Set g_app to nullptr after the application destroyed to help the
        // objects decide whether there's need to do the clearing.

        g_app = nullptr;
    }

    void Application::initWin32Window()
    {
        auto hInstance = GetModuleHandle(nullptr);

        //////////////////
        // Window Class //
        //////////////////

        WNDCLASSEX wndclass =
        {
            .cbSize        = sizeof(wndclass),
            .style         = CS_DBLCLKS,
            .lpfnWndProc   = fnWndProc,
            .cbWndExtra    = sizeof(this), // g_app
            .hInstance     = hInstance,
            .lpszClassName = createInfo.name.c_str()
        };
        RegisterClassEx(&wndclass);

        /////////////////////
        // Size & Position //
        /////////////////////

        RECT workAreaRect = {};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);

        auto workAreaWidth = math_utils::width(workAreaRect);
        auto workAreaHeight = math_utils::height(workAreaRect);

        auto wndrect = math_utils::centered
        (
        /* dst */ { 0, 0, workAreaWidth, workAreaHeight },
        /* src */ platform_utils::scaledByDpi(createInfo.windowSize)
        );
        DWORD dwStyle = WS_POPUP;
        // Prevent DWM from drawing the window again.
        DWORD dwExStyle = WS_EX_NOREDIRECTIONBITMAP;

        auto dpi = (UINT)platform_utils::dpi();
        AdjustWindowRectExForDpi(&wndrect, dwStyle, FALSE, dwExStyle, dpi);

        //////////////////
        // Win32 Window //
        //////////////////

        m_win32Window = CreateWindowEx
        (
        /* dwExStyle    */ dwExStyle,
        /* lpClassName  */ createInfo.name.c_str(),
        /* lpWindowName */ createInfo.name.c_str(),
        /* dwStyle      */ dwStyle,
        /* X            */ wndrect.left,
        /* Y            */ wndrect.top,
        /* nWidth       */ math_utils::width(wndrect),
        /* nHeight      */ math_utils::height(wndrect),
        /* hWndParent   */ nullptr,
        /* hMenu        */ nullptr,
        /* hInstance    */ hInstance,
        /* lpParam      */ nullptr
        );
        THROW_IF_NULL(m_win32Window);

        ///////////////
        // User Data //
        ///////////////

        SetWindowLongPtr(m_win32Window, GWLP_USERDATA, (LONG_PTR)this);
    }

    void Application::initDirectX12Renderer()
    {
        auto dpi = platform_utils::dpi();

        ///////////////////
        // Main Renderer //
        ///////////////////

        DWORD displayAffinity = WDA_NONE;
        if (createInfo.excludeFromCapture)
        {
            displayAffinity = WDA_EXCLUDEFROMCAPTURE;
        }
        Renderer::CreateInfo info =
        {
            .binaryPath      = createInfo.binaryPath,
            .dpi             = dpi,
            .fullscreen      = createInfo.fullscreen,
            .displayAffinity = displayAffinity,
            .duplication     = createInfo.duplication,
            .composition     = createInfo.composition
        };
        m_renderer = std::make_unique<Renderer>(m_win32Window, info);

        /////////////////////
        // Direct2D Config //
        /////////////////////

        auto context = m_renderer->d2d1DeviceContext();

        context->SetDpi(dpi, dpi);
        context->SetUnitMode(D2D1_UNIT_MODE_DIPS);

        /////////////////////
        // Resource Config //
        /////////////////////

        m_renderer->skipUpdating = true;
        m_renderer->timer()->pause();

        ////////////////////
        // UI Render Pass //
        ////////////////////

        auto device = m_renderer->d3d12Device();
        m_uiCmdLayer = std::make_shared<UICommandLayer>(device);
        {
            m_uiCmdLayer->setPriority(g_uiCmdLayerPriority);
            m_uiCmdLayer->drawTarget.emplace<UIDrawTarget>();
        }
        m_renderer->cmdLayers.insert(m_uiCmdLayer);
    }

    void Application::initMiscComponents()
    {
        m_themeStyle = querySystemThemeStyle();
        // appearance::initialize depends on Application::m_themeStyle
        appearance::initialize();

        if (m_themeStyle.name == L"Light")
        {
            m_renderer->setSceneColor(Colors::White);
        }
        else if (m_themeStyle.name == L"Dark")
        {
            m_renderer->setSceneColor(Colors::Black);
        }
        resource_utils::initialize();

        m_cursor = makeUIObject<Cursor>();
        m_cursor->setPrivateVisible(false);
    }

    int Application::run(FuncRefer<void(Application* app)> onLaunch)
    {
        if (onLaunch) onLaunch(this);

        m_renderer->renderNextFrame();

        ShowWindow(m_win32Window, SW_SHOW);
        UpdateWindow(m_win32Window);

        MSG msg = {};
        while (true)
        {
            if (m_animationCount == 0)
            {
                auto ret = GetMessage(&msg, nullptr, 0, 0);

                // (WM_QUIT  || An Error )
                if (ret == 0 || ret == -1)
                {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else // full speed rendering
            {
                if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                    {
                        break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                // use "else" here to clear the message queue
                else m_renderer->renderNextFrame();
            }
        }
        return (int)msg.wParam;
    }

    void Application::exit()
    {
        SendMessage(m_win32Window, WM_DESTROY, 0, 0);
    }

    HWND Application::win32Window() const
    {
        return m_win32Window;
    }

    LRESULT CALLBACK Application::fnWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (RuntimeError::g_flag)
        {
            // A RuntimeError has already been thrown (RuntimeError::g_flag=True),
            // so there is no need to process the remaining messages in the queue.
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        auto app = (Application*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (message)
        {
        case WM_SIZE:
        {
            if (app != nullptr)
            {
                SIZE clntSize = { LOWORD(lParam), HIWORD(lParam) };
                if (clntSize.cx > 0 && clntSize.cy > 0)
                {
                    app->m_renderer->onWindowResize();

                    if (app->win32WindowSettings.callback.f_onClientAreaSize)
                    {
                        app->win32WindowSettings.callback.f_onClientAreaSize(clntSize);
                    }
                    app->m_renderer->renderNextFrame();
                }
            }
            return 0;
        }
        case WM_PAINT:
        {
            if (app != nullptr)
            {
                app->m_renderer->renderNextFrame();

                // Prevent the system from sending WM_PAINT repeatedly.
                ValidateRect(hwnd, nullptr);
            }
            return 0;
        }
        case WM_ERASEBKGND:
        {
            return TRUE; // Prevent GDI from clearing background.
        }
        case WM_SETTINGCHANGE:
        {
            if (app != nullptr)
            {
                if (lstrcmp(LPCTSTR(lParam), L"ImmersiveColorSet") == 0)
                {
                    auto style = Application::querySystemThemeStyle();

                    if (app->themeStyle() != style && app->f_onSystemThemeStyleChange)
                    {
                        app->f_onSystemThemeStyleChange(style);
                    }
                }
            }
            return 0;
        }
        case WM_ACTIVATEAPP:
        {
            if (app != nullptr)
            {
                if (wParam && IsMinimized(app->m_win32Window))
                {
                    if (app->win32WindowSettings.callback.f_onRestoreFromMinimized)
                    {
                        app->win32WindowSettings.callback.f_onRestoreFromMinimized();
                    }
                }
                if (!wParam && app->m_renderer->window().fullscreen())
                {
                    // Make sure the fullscreen-window can be switched by Alt+Tab.
                    ShowWindow(app->m_win32Window, SW_MINIMIZE);
                }
            }
            return 0;
        }
        case WM_SETCURSOR:
        {
            if (app != nullptr && LOWORD(lParam) == HTCLIENT)
            {
                if (app->m_cursor->m_iconSource == Cursor::System)
                {
                    app->m_cursor->setSystemIcon();
                }
                else SetCursor(nullptr); // Take over cursor drawing from GDI.

                // reset this for next candidate WM_SETCURSOR message
                app->m_cursor->m_systemIconUpdateFlag = false;

                // If an application processes this message,
                // it should return TRUE to halt further processing.
                return TRUE;
            }
            // The non-client area hit is handed over to the system for processing.
            // (used to resize the Win32 Window, so display the system cursors)
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        case WM_GETMINMAXINFO:
        {
            // We must config the min-max-info for the popup-window manually,
            // otherwise it occupies the whole screen when showing maximized.

            auto pMinMaxInfo = (MINMAXINFO*)lParam;

            RECT workAreaRect = {};
            SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);

            pMinMaxInfo->ptMaxSize.x = math_utils::width(workAreaRect);
            pMinMaxInfo->ptMaxSize.y = math_utils::height(workAreaRect);

            pMinMaxInfo->ptMaxPosition.x = workAreaRect.left;
            pMinMaxInfo->ptMaxPosition.y = workAreaRect.top;

            if (app != nullptr && app->win32WindowSettings.geometry.minTrackSize.has_value())
            {
                auto& minSize = app->win32WindowSettings.geometry.minTrackSize.value();
                pMinMaxInfo->ptMinTrackSize = platform_utils::scaledByDpi(minSize);
            }
            return 0;
        }
        case WM_NCHITTEST:
        {
            if (app != nullptr)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient(hwnd, &pt);

                return app->handleWin32NCHITTESTMessage(platform_utils::restoredByDpi(pt));
            }
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        case WM_MOUSEMOVE:
        {
            if (app == nullptr || app->m_isHandlingSensitiveUIEvent)
            {
                return 0; // Only one sensitive UI event can be handled at a time.
            }
            app->m_isHandlingSensitiveUIEvent = true;

            D2D1_POINT_2F cursorPoint =
            {
                (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)
            };
            cursorPoint = platform_utils::restoredByDpi(cursorPoint);

            if (!app->isTriggerDraggingWin32Window)
            {
                // The cursor position may jitter When dragging the Win32 window.
                app->m_cursor->setPosition(cursorPoint.x, cursorPoint.y);
            }
            app->m_cursor->setIcon(Cursor::Arrow);

            MouseMoveEvent e = {};
            {
                e.cursorPoint = cursorPoint;

                e.buttonState.leftPressed = wParam & MK_LBUTTON;
                e.buttonState.middlePressed = wParam & MK_MBUTTON;
                e.buttonState.rightPressed = wParam & MK_RBUTTON;

                e.keyState.ALT = wParam & MK_ALT;
                e.keyState.CTRL = wParam & MK_CONTROL;
                e.keyState.SHIFT = wParam & MK_SHIFT;

                e.lastCursorPoint = app->m_lastCursorPoint;
                app->m_lastCursorPoint = e.cursorPoint;
            }
            //------------------------------------------------------------------
            // START: Mouse-Move Event
            //------------------------------------------------------------------

            ISortable<Panel>::foreach(app->m_pinnedUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
            {
                if (uiobj->appEventReactability.mouse.move)
                {
                    uiobj->onMouseMove(e);
                }
                return uiobj->appEventTransparency.mouse.move;
            });
            auto& mouseFocused = app->m_focusedUIObjects[(size_t)FocusType::Mouse];

            if (!mouseFocused.expired())
            {
                mouseFocused.lock()->onMouseMove(e);
            }
            else // Deliver mouse-move event normally.
            {
                UIObjectTempSet hitUIObjects = {};

                for (auto& uiobj : app->m_uiObjects)
                {
                    if (uiobj->appEventReactability.hitTest && uiobj->isHit(cursorPoint))
                    {
                        hitUIObjects.insert(uiobj);
                    }
                }
                if (app->forceSingleMouseEnterLeaveEvent)
                {
                    WeakPtr<Panel> enterCandidate = {}, leaveCandidate = {};
                    if (!hitUIObjects.empty())
                    {
                        enterCandidate = *hitUIObjects.begin();
                    }
                    if (!app->m_hitUIObjects.empty())
                    {
                        leaveCandidate = *app->m_hitUIObjects.begin();
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
                    ISortable<Panel>::foreach(hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                    {
                        // Moved in just now, trigger mouse-enter event.
                        if (app->m_hitUIObjects.find(uiobj) == app->m_hitUIObjects.end())
                        {
                            if (uiobj->appEventReactability.mouse.enter)
                            {
                                uiobj->onMouseEnter(e);
                            }
                            return uiobj->appEventTransparency.mouse.enter;
                        }
                        return true;
                    });
                    ISortable<Panel>::foreach(app->m_hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                    {
                        // Moved out just now, trigger mouse-leave event.
                        if (hitUIObjects.find(uiobj) == hitUIObjects.end())
                        {
                            if (uiobj->appEventReactability.mouse.leave)
                            {
                                uiobj->onMouseLeave(e);
                            }
                            return uiobj->appEventTransparency.mouse.leave;
                        }
                        return true;
                    });
                }
                app->m_hitUIObjects = std::move(hitUIObjects);

                ISortable<Panel>::foreach(app->m_hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                {
                    if (uiobj->appEventReactability.mouse.move)
                    {
                        uiobj->onMouseMove(e);
                    }
                    return uiobj->appEventTransparency.mouse.move;
                });
            }
            //------------------------------------------------------------------
            // END: Mouse-Move Event
            //------------------------------------------------------------------

            // The cursor will be hidden if moves out of the Win32 window,
            // so we need to show it explicitly in every mouse-move event.
            app->m_cursor->setPrivateVisible(true);

            if (app->m_cursor->m_iconSource == Cursor::System)
            {
                app->m_cursor->setSystemIcon();
            }
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme); // This is required for WM_MOUSELEAVE.

            InvalidateRect(hwnd, nullptr, FALSE);

            app->m_isHandlingSensitiveUIEvent = false;
            return 0;
        }
        case WM_MOUSELEAVE:
        {
            if (app == nullptr || app->m_isHandlingSensitiveUIEvent)
            {
                return 0; // Only one sensitive UI event can be handled at a time.
            }
            app->m_isHandlingSensitiveUIEvent = true;

            POINT cursorPoint = {};
            GetCursorPos(&cursorPoint);
            ScreenToClient(hwnd, &cursorPoint);

            cursorPoint = platform_utils::restoredByDpi(cursorPoint);

            MouseMoveEvent e = {};
            {
                e.cursorPoint =
                {
                    (float)cursorPoint.x, (float)cursorPoint.y
                };
                e.lastCursorPoint = app->m_lastCursorPoint;
                app->m_lastCursorPoint = e.cursorPoint;
            }
            //------------------------------------------------------------------
            // START: Mouse-Leave Event
            //------------------------------------------------------------------

            auto& mouseFocused = app->m_focusedUIObjects[(size_t)FocusType::Mouse];

            if (!mouseFocused.expired())
            {
                mouseFocused.lock()->onMouseLeave(e);
            }
            else // Deliver mouse-leave event normally.
            {
                ISortable<Panel>::foreach(app->m_hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                {
                    if (uiobj->appEventReactability.mouse.leave)
                    {
                        uiobj->onMouseLeave(e);
                    }
                    return uiobj->appEventTransparency.mouse.leave;
                });
                app->m_hitUIObjects.clear();
            }
            //------------------------------------------------------------------
            // END: Mouse-Leave Event
            //------------------------------------------------------------------

            app->m_cursor->setPrivateVisible(false);

            InvalidateRect(hwnd, nullptr, FALSE);

            app->m_isHandlingSensitiveUIEvent = false;
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        {
            SetCapture(hwnd);
            goto handle_mouse_button_event;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            ReleaseCapture();
            goto handle_mouse_button_event;
        }
        handle_mouse_button_event:
        {
            if (app == nullptr || app->m_isHandlingSensitiveUIEvent)
            {
                return 0; // Only one sensitive UI event can be handled at a time.
            }
            app->m_isHandlingSensitiveUIEvent = true;

            MouseButtonEvent e = {};
            {
                D2D1_POINT_2F cursorPoint =
                {
                    (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)
                };
                e.cursorPoint = platform_utils::restoredByDpi(cursorPoint);

                e.state.flag = MouseButtonEvent::State::g_flagMap.at(message);
            }
            //------------------------------------------------------------------
            // START: Mouse-Button Event
            //------------------------------------------------------------------

            ISortable<Panel>::foreach(app->m_pinnedUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
            {
                if (uiobj->appEventReactability.mouse.button)
                {
                    uiobj->onMouseButton(e);
                }
                return uiobj->appEventTransparency.mouse.button;
            });
            auto& mouseFocused = app->m_focusedUIObjects[(size_t)FocusType::Mouse];

            if (!mouseFocused.expired())
            {
                mouseFocused.lock()->onMouseButton(e);
            }
            else // Deliver mouse-button event normally.
            {
                ISortable<Panel>::foreach(app->m_hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                {
                    if (uiobj->appEventReactability.mouse.button)
                    {
                        uiobj->onMouseButton(e);
                    }
                    return uiobj->appEventTransparency.mouse.button;
                });
                app->handleImmediateMouseMoveEventCallback();
            }
            //------------------------------------------------------------------
            // END: Mouse-Button Event
            //------------------------------------------------------------------

            InvalidateRect(hwnd, nullptr, FALSE);

            app->m_isHandlingSensitiveUIEvent = false;
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            if (app == nullptr || app->m_isHandlingSensitiveUIEvent)
            {
                return 0; // Only one sensitive UI event can be handled at a time.
            }
            app->m_isHandlingSensitiveUIEvent = true;

            POINT cursorPoint =
            {
                GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)
            };
            ScreenToClient(hwnd, &cursorPoint);

            cursorPoint = platform_utils::restoredByDpi(cursorPoint);

            MouseWheelEvent e = {};
            {
                e.cursorPoint =
                {
                    (float)cursorPoint.x, (float)cursorPoint.y
                };
                auto lowParam = LOWORD(wParam);

                e.buttonState.leftPressed = lowParam & MK_LBUTTON;
                e.buttonState.middlePressed = lowParam & MK_MBUTTON;
                e.buttonState.rightPressed = lowParam & MK_RBUTTON;

                e.keyState.CTRL = lowParam & MK_CONTROL;
                e.keyState.SHIFT = lowParam & MK_SHIFT;

                e.deltaCount = GET_Y_LPARAM(wParam) / WHEEL_DELTA;
            }
            //------------------------------------------------------------------
            // START: Mouse-Wheel Event
            //------------------------------------------------------------------

            ISortable<Panel>::foreach(app->m_pinnedUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
            {
                if (uiobj->appEventReactability.mouse.wheel)
                {
                    uiobj->onMouseWheel(e);
                }
                return uiobj->appEventTransparency.mouse.wheel;
            });
            auto& mouseFocused = app->m_focusedUIObjects[(size_t)FocusType::Mouse];

            if (!mouseFocused.expired())
            {
                mouseFocused.lock()->onMouseWheel(e);
            }
            else // Deliver mouse-wheel event normally.
            {
                ISortable<Panel>::foreach(app->m_hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                {
                    if (uiobj->appEventReactability.mouse.wheel)
                    {
                        uiobj->onMouseWheel(e);
                    }
                    return uiobj->appEventTransparency.mouse.wheel;
                });
                app->handleImmediateMouseMoveEventCallback();
            }
            //------------------------------------------------------------------
            // END: Mouse-Wheel Event
            //------------------------------------------------------------------

            InvalidateRect(hwnd, nullptr, FALSE);

            app->m_isHandlingSensitiveUIEvent = false;
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if (app == nullptr || app->m_isHandlingSensitiveUIEvent)
            {
                return 0; // Only one sensitive UI event can be handled at a time.
            }
            app->m_isHandlingSensitiveUIEvent = true;

            KeyboardEvent e = {};
            {
                e.vkey = wParam;

                if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
                {
                    e.state.flag = KeyboardEvent::State::Flag::Pressed;
                }
                else e.state.flag = KeyboardEvent::State::Flag::Released;
            }
            //------------------------------------------------------------------
            // START: Keyboard Event
            //------------------------------------------------------------------

            ISortable<Panel>::foreach(app->m_pinnedUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
            {
                if (uiobj->appEventReactability.keyboard)
                {
                    uiobj->onKeyboard(e);
                }
                return uiobj->appEventTransparency.keyboard;
            });
            auto& keyboardFocused = app->m_focusedUIObjects[(size_t)FocusType::Keyboard];

            if (!keyboardFocused.expired())
            {
                keyboardFocused.lock()->onKeyboard(e);
            }
            else // Deliver keyboard event normally.
            {
                ISortable<Panel>::foreach(app->m_hitUIObjects, [&](ShrdPtrRefer<Panel> uiobj)
                {
                    if (uiobj->appEventReactability.keyboard)
                    {
                        uiobj->onKeyboard(e);
                    }
                    return uiobj->appEventTransparency.keyboard;
                });
                app->handleImmediateMouseMoveEventCallback();
            }
            //------------------------------------------------------------------
            // END: Keyboard Event
            //------------------------------------------------------------------

            InvalidateRect(hwnd, nullptr, FALSE);

            app->m_isHandlingSensitiveUIEvent = false;
            return 0;
        }
#define HANDLE_TEXT_INPUT_OBJECT_START(Name) \
if (app != nullptr) \
{ \
    auto tiobj = app->getFocusedTextInputObject(); \
    if (!tiobj.expired()) \
    { \
        auto Name = tiobj.lock(); \

#define HANDLE_TEXT_INPUT_OBJECT_END \
    } \
}
        case WM_CHAR:
        {
            HANDLE_TEXT_INPUT_OBJECT_START(ptiobj)

            ptiobj->OnInputString({ (WCHAR*)&wParam, 1 });
            InvalidateRect(hwnd, nullptr, FALSE);

            HANDLE_TEXT_INPUT_OBJECT_END
            return 0;
        }
        case WM_IME_STARTCOMPOSITION:
        {
            HANDLE_TEXT_INPUT_OBJECT_START(ptiobj)

            auto himc = ImmGetContext(hwnd);
            if (himc != nullptr)
            {
                auto font = ptiobj->getCompositionFont();
                if (font.has_value())
                {
                    ImmSetCompositionFont(himc, &font.value());
                }
                auto form = ptiobj->getCompositionForm();
                if (form.has_value())
                {
                    ImmSetCompositionWindow(himc, &form.value());
                }
            }
            ImmReleaseContext(hwnd, himc);

            HANDLE_TEXT_INPUT_OBJECT_END

            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        case WM_IME_COMPOSITION:
        {
            if (lParam & GCS_RESULTSTR)
            {
                HANDLE_TEXT_INPUT_OBJECT_START(ptiobj)

                auto himc = ImmGetContext(hwnd);
                if (himc != nullptr)
                {
                    auto nSize = ImmGetCompositionString(himc, GCS_RESULTSTR, nullptr, 0);
                    if (nSize > 0)
                    {
                        // The null-terminated needs an extra character space.
                        auto hLocal = LocalAlloc(LPTR, nSize + sizeof(WCHAR));
                        if (hLocal != nullptr)
                        {
                            ImmGetCompositionString(himc, GCS_RESULTSTR, hLocal, nSize);
                            ptiobj->OnInputString((WCHAR*)hLocal);
                            InvalidateRect(hwnd, nullptr, FALSE);
                        }
                        LocalFree(hLocal);
                    }
                }
                ImmReleaseContext(hwnd, himc);

                HANDLE_TEXT_INPUT_OBJECT_END

                // Prevent from receiving WM_CHAR with the result string since it has been handled above.
                lParam &= ~(GCS_RESULTCLAUSE | GCS_RESULTREADCLAUSE | GCS_RESULTREADSTR | GCS_RESULTSTR);
            }
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
#undef HANDLE_TEXT_INPUT_OBJECT_START
#undef HANDLE_TEXT_INPUT_OBJECT_END

        case (UINT)CustomMessage::AwakeGetMessage:
        {
            return 0;
        }
        case (UINT)CustomMessage::HandleThreadEvent:
        {
            if (app != nullptr)
            {
                auto id = (ThreadEventID)wParam;

                auto callback = app->m_threadCallbacks.find(id);
                if (callback != app->m_threadCallbacks.end())
                {
                    callback->second((ThreadEventData)lParam);
                }
            }
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        default: return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    LRESULT Application::handleWin32NCHITTESTMessage(const POINT& pt)
    {
        if (IsMaximized(m_win32Window))
        {
            return HTCLIENT;
        }
        if (win32WindowSettings.sizingFrame.f_NCHITTEST)
        {
            return win32WindowSettings.sizingFrame.f_NCHITTEST(pt);
        }
        return defWin32NCHITTESTMessageHandler(pt);
    }

    LRESULT Application::defWin32NCHITTESTMessageHandler(const POINT& pt)
    {
        if (win32WindowSettings.sizingFrame.frameWidth.has_value())
        {
            auto frameWidth = win32WindowSettings.sizingFrame.frameWidth.value();

            RECT clntRect = {};
            GetClientRect(m_win32Window, &clntRect);

            auto clntSize = platform_utils::restoredByDpi(math_utils::size(clntRect));

            if (pt.x <= frameWidth)
            {
                if (pt.y <= frameWidth)
                {
                    return HTTOPLEFT;
                }
                if (pt.y >= clntSize.cy - frameWidth)
                {
                    return HTBOTTOMLEFT;
                }
                return HTLEFT;
            }
            if (pt.x >= clntSize.cx - frameWidth)
            {
                if (pt.y <= frameWidth)
                {
                    return HTTOPRIGHT;
                }
                if (pt.y >= clntSize.cy - frameWidth)
                {
                    return HTBOTTOMRIGHT;
                }
                return HTRIGHT;
            }
            if (pt.y <= frameWidth)
            {
                if (pt.x <= frameWidth)
                {
                    return HTTOPLEFT;
                }
                if (pt.x >= clntSize.cx - frameWidth)
                {
                    return HTTOPRIGHT;
                }
                return HTTOP;
            }
            if (pt.y >= clntSize.cy - frameWidth)
            {
                if (pt.x <= frameWidth)
                {
                    return HTBOTTOMLEFT;
                }
                if (pt.x >= clntSize.cx - frameWidth)
                {
                    return HTBOTTOMRIGHT;
                }
                return HTBOTTOM;
            }
        }
        return HTCLIENT;
    }

    Renderer* Application::dx12Renderer() const
    {
        return m_renderer.get();
    }

    Application::UIDrawTarget& Application::drawObjects()
    {
        // m_uiCmdLayer->drawTarget is guaranteed to be a UIDrawTarget.
        return std::get<UIDrawTarget>(m_uiCmdLayer->drawTarget);
    }

    const SharedPtr<Renderer::CommandLayer>& Application::uiCmdLayer() const
    {
        return m_uiCmdLayer;
    }

    void Application::registerDrawObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        if (drawObjects().find(uiobj) != drawObjects().end()) return;

        /////////////////////////
        // Update Draw Objects //
        /////////////////////////

        drawObjects().insert(uiobj);

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

    void Application::unregisterDrawObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;

        /////////////////////////
        // Update Draw Objects //
        /////////////////////////

        drawObjects().erase(uiobj);

        ///////////////////////
        // Update Priorities //
        ///////////////////////

        // There is no need to update the priorities here,
        // as newly registered objects will automatically
        // adapt to the existing priorities.
    }

#pragma warning(push)
// wrappedBuffer is guaranteed to be valid when composition=False
#pragma warning(disable : 26815)

    ComPtr<ID2D1Bitmap1> Application::windowshot() const
    {
//--------------------------------------------------------------------------
// (composition=True)
// renderTarget is the first back buffer of the composition swapChain,
// which internally performs synchronization, so direct copying is fine.
//--------------------------------------------------------------------------
// (composition=False)
// renderTarget is a D2D1Bitmap created from wrappedBuffer (D3D11Resource),
// which is created from sceneBuffer (D3D12Resource) through D3D11On12Device,
// so it is necessary to synchronize D3D12 and D3D11.
//--------------------------------------------------------------------------

        /////////////
        // Acquire //
        /////////////

        if (!m_renderer->composition())
        {
            auto wrapped = m_renderer->wrappedBuffer().value();
            m_renderer->d3d11On12Device()->AcquireWrappedResources(&wrapped, 1);
        }
        /////////////
        // Copying //
        /////////////

        auto src = m_renderer->renderTarget();
        auto pixSize = src->GetPixelSize();

        auto dst = bitmap_utils::loadBitmap(pixSize.width, pixSize.height);

        D2D1_POINT_2U dstPoint = { 0, 0 };
        D2D1_RECT_U srcRect = { 0, 0, pixSize.width, pixSize.height };
        THROW_IF_FAILED(dst->CopyFromBitmap(&dstPoint, src, &srcRect));

        /////////////
        // Release //
        /////////////

        if (!m_renderer->composition())
        {
            auto wrapped = m_renderer->wrappedBuffer().value();
            m_renderer->d3d11On12Device()->ReleaseWrappedResources(&wrapped, 1);
        }
        return dst;
    }
#pragma warning(pop)

    Optional<ComPtr<ID2D1Bitmap1>> Application::screenshot() const
    {
        m_renderer->tryUpdateDuplFrame();

        return m_renderer->duplBitmap();
    }

    UINT Application::animationCount() const
    {
        return m_animationCount;
    }

    void Application::increaseAnimationCount()
    {
        if (m_animationCount < UINT_MAX)
        {
            ++m_animationCount;
        }
        if (m_animationCount == 1)
        {
            m_renderer->skipUpdating = false;
            m_renderer->timer()->resume();

            postCustomMessage(CustomMessage::AwakeGetMessage);
        }
    }

    void Application::decreaseAnimationCount()
    {
        if (m_animationCount > 0)
        {
            --m_animationCount;
        }
        if (m_animationCount == 0)
        {
            m_renderer->skipUpdating = true;
            m_renderer->timer()->start();
            m_renderer->timer()->pause();
        }
    }

    const Application::UIObjectSet& Application::uiObjects() const
    {
        return m_uiObjects;
    }

    void Application::registerUIEvents(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        if (m_uiObjects.find(uiobj) != m_uiObjects.end()) return;

        ///////////////////////
        // Update UI Objects //
        ///////////////////////

        m_uiObjects.insert(uiobj);

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
    }

    void Application::unregisterUIEvents(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;

        ///////////////////////
        // Update UI Objects //
        ///////////////////////

        m_uiObjects.erase(uiobj);

        ///////////////////////
        // Update Priorities //
        ///////////////////////

        // There is no need to update the priorities here,
        // as newly registered objects will automatically
        // adapt to the existing priorities.
    }

    void Application::addUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        registerDrawObject(uiobj);
        registerUIEvents(uiobj);
    }

    void Application::removeUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        unregisterDrawObject(uiobj);
        unregisterUIEvents(uiobj);
    }

    void Application::clearAddedUIObjects()
    {
        drawObjects().clear();
        m_uiObjects.clear();

        m_frontPriorities = {};
        m_backPriorities = {};
    }

    void Application::pinUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        m_pinnedUIObjects.insert(uiobj);
    }

    void Application::unpinUIObject(ShrdPtrRefer<Panel> uiobj)
    {
        if (uiobj == nullptr) return;
        m_pinnedUIObjects.erase(uiobj);
    }

    void Application::clearPinnedUIObjects()
    {
        m_pinnedUIObjects.clear();
    }

    void Application::focusUIObject(FocusType focus, ShrdPtrRefer<Panel> uiobj)
    {
        auto& focused = m_focusedUIObjects[(size_t)focus];
        if (cpp_lang_utils::isMostDerivedEqual(uiobj, focused.lock())) return;

        switch (focus)
        {
        case FocusType::Mouse:
        {
            if (!focused.expired())
            {
                focused.lock()->onLoseMouseFocus();
            }
            focused = uiobj;

            if (!focused.expired())
            {
                focused.lock()->onGetMouseFocus();
            }
            break;
        }
        case FocusType::Keyboard:
        {
            if (!focused.expired())
            {
                focused.lock()->onLoseKeyboardFocus();
            }
            focused = uiobj;

            if (!focused.expired())
            {
                focused.lock()->onGetKeyboardFocus();
            }
            break;
        }
        default: break;
        }
    }

    void Application::handleImmediateMouseMoveEventCallback()
    {
        if (sendNextImmediateMouseMoveEvent)
        {
            sendNextImmediateMouseMoveEvent = false;

            POINT cursorPoint = {};
            GetCursorPos(&cursorPoint);
            ScreenToClient(m_win32Window, &cursorPoint);

            WPARAM wParam = 0;
            LPARAM lParam = MAKELPARAM(cursorPoint.x, cursorPoint.y);

            PostMessage(m_win32Window, WM_MOUSEMOVE, wParam, lParam);
        }
    }

    WeakPtr<TextInputObject> Application::getFocusedTextInputObject() const
    {
        auto& uiobj = m_focusedUIObjects[(size_t)FocusType::Keyboard];
        return std::dynamic_pointer_cast<TextInputObject>(uiobj.lock());
    }

    const Application::PriorityGroup& Application::frontPriorities() const
    {
        return m_frontPriorities;
    }

    const Application::PriorityGroup& Application::backPriorities() const
    {
        return m_backPriorities;
    }

    void Application::bringRootObjectToFront(Panel* uiobj)
    {
        uiobj->setDrawObjectPriority(++m_frontPriorities.drawObject);
        uiobj->setUIObjectPriority(--m_frontPriorities.uiObject);
    }

    void Application::sendRootObjectToBack(Panel* uiobj)
    {
        uiobj->setDrawObjectPriority(--m_backPriorities.drawObject);
        uiobj->setUIObjectPriority(++m_backPriorities.uiObject);
    }

    Cursor* Application::cursor() const
    {
        return m_cursor.get();
    }

    const D2D1_POINT_2F& Application::lastCursorPoint() const
    {
        return m_lastCursorPoint;
    }

    Wstring Application::querySystemThemeName()
    {
        DWORD data = TRUE;
        DWORD dataSize = sizeof(data);

        THROW_IF_FAILED(RegGetValue
        (
        /* hkey     */ HKEY_CURRENT_USER,
        /* lpSubKey */ L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        /* lpValue  */ L"AppsUseLightTheme",
        /* dwFlags  */ RRF_RT_REG_DWORD,
        /* pdwType  */ nullptr,
        /* pvData   */ &data,
        /* pcbData  */ &dataSize
        ));
        return data ? L"Light" : L"Dark";
    }

    D2D1_COLOR_F Application::querySystemThemeColor()
    {
        DWORD data = 0x00'00'00'00;
        DWORD dataSize = sizeof(data);

        THROW_IF_FAILED(RegGetValue
        (
        /* hkey     */ HKEY_CURRENT_USER,
        /* lpSubKey */ L"Software\\Microsoft\\Windows\\DWM",
        /* lpValue  */ L"AccentColor",
        /* dwFlags  */ RRF_RT_REG_DWORD,
        /* pdwType  */ nullptr,
        /* pvData   */ &data,
        /* pcbData  */ &dataSize
        ));
        return (D2D1_COLOR_F)color_utils::ABGR(data);
    }

    Application::ThemeStyle Application::querySystemThemeStyle()
    {
        return { querySystemThemeName(), querySystemThemeColor() };
    }

    const Application::ThemeStyle& Application::themeStyle() const
    {
        return m_themeStyle;
    }

    void Application::setThemeStyle(const ThemeStyle& style)
    {
        appearance::generateTonedColors(style);

        if (style.name == L"Light")
        {
            m_renderer->setSceneColor(Colors::White);
        }
        else if (style.name == L"Dark")
        {
            m_renderer->setSceneColor(Colors::Black);
        }
        for (auto& uiobj : m_uiObjects)
        {
            if (uiobj->enableChangeThemeStyleUpdate)
            {
                uiobj->onChangeThemeStyle(style);
            }
        }
        m_themeStyle = style;
    }

    const Wstring& Application::langLocale() const
    {
        return m_langLocale;
    }

    void Application::setLangLocale(WstrRefer codeName)
    {
        for (auto& uiobj : m_uiObjects)
        {
            if (uiobj->enableChangeLangLocaleUpdate)
            {
                uiobj->onChangeLangLocale(codeName);
            }
        }
        m_langLocale = codeName;
    }

    void Application::postCustomMessage(CustomMessage message, WPARAM wParam, LPARAM lParam)
    {
        PostMessage(m_win32Window, (UINT)message, wParam, lParam);
    }

    void Application::registerThreadCallback(ThreadEventID id, ThreadCallbackParam callback)
    {
        m_threadCallbacks[id] = callback;
    }

    void Application::unregisterThreadCallback(ThreadEventID id)
    {
        m_threadCallbacks.erase(id);
    }

    void Application::triggerThreadEvent(ThreadEventID id, ThreadEventData data)
    {
        postCustomMessage(CustomMessage::HandleThreadEvent, id, data);
    }
}
