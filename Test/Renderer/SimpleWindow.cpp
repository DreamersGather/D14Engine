#include "Common/Precompile.h"

#include "Common/MathUtils/Basic.h"
#include "Common/MathUtils/GDI.h"
#include "Common/RuntimeError.h"

#include "Renderer/InfoUtils.h"
#include "Renderer/Renderer.h"

using namespace d14engine;
using namespace d14engine::renderer;

#define DEMO_NAME L"D14Engine - SimpleWindow @ Renderer"

std::unique_ptr<Renderer> initApp(UINT, UINT, Renderer::CreateInfo&);

int wmain(int argc, wchar_t* argv[])
{
    try // D14Engine - SimpleWindow @ Renderer
    {
        // Place this setting at the very beginning to ensure that the MessageBox
        // with relevant information for initialization errors also supports HiDPI.
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        Wstring exePath(MAX_PATH, 0);
        GetModuleFileName(nullptr, exePath.data(), MAX_PATH);
        // There is no need to check the result of find_last_of
        // because the exe-path is guaranteed to contain a "\".
        exePath.resize(exePath.find_last_of(L'\\') + 1);

        // Set this before calling AddDllDirectory to ensure that
        // subsequent LoadLibrary calls search user-defined paths.
        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

        std::wstring libPaths[] =
        {
            L"Lib/DirectXShaderCompiler"
        };
#ifdef _WIN64
        std::wstring arch = L"x64";
#else
        std::wstring arch = L"x86";
#endif
        for (auto& libPath : libPaths)
        {
            // Special note: AddDllDirectory only accepts absolute paths!
            THROW_IF_NULL(AddDllDirectory((exePath + libPath + L"/" + arch).c_str()));
        }
        //-------------------------------------------------------------
        //--------------------------- Usage ---------------------------
        //-------------------------------------------------------------
        // The window is in free scaling mode by default:
        //
        // 1. Press [Space] to switch between different display modes,
        //    in which cases the letterbox will be used for stretching.
        //
        // 2. Press [Backspace] to turn on/off free scaling mode.
        //-------------------------------------------------------------
        // The window is rendered with the letterbox by default:
        //
        // 1. Press [Enter] to enable/disable the composition layer:
        //
        //    (composition=True) The letterbox will be released,
        //    and the window is rendered with the composition layer,
        //    in which case the background color can be transparent.
        //
        //    (composition=False) The letterbox will be created,
        //    and the window is rendered with the letterbox scene,
        //    in which case the background color is not transparent.
        //-------------------------------------------------------------
        // NOTE: When using the composition layer, the window is
        //       always in free scalilng mode, and the layer bitmap
        //       costs extra GPU commands for alpha blending.
        //       It is recommended to disable the composition layer
        //       if the application is designed for high-performance
        //       rendering without any transparent background.
        //-------------------------------------------------------------

        Renderer::CreateInfo info = {};
        info.sceneColor = Colors::SteelBlue;
        
        auto rndr = initApp(800, 600, info);

        ShowWindow(rndr->window().ptr, SW_SHOW);
        UpdateWindow(rndr->window().ptr);

        MSG msg;
        while (true)
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
            else rndr->renderNextFrame();
        }
        return (int)msg.wParam;
    }
    catch (std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "std::exception", MB_OK | MB_ICONERROR);
    }
    catch (d14engine::RuntimeError& e)
    {
        MessageBoxW(nullptr, e.message().c_str(), L"d14engine::RuntimeError", MB_OK | MB_ICONERROR);
    }
    catch (...) // fallthrough
    {
        MessageBoxW(nullptr, L"Unknown exception encountered.", L"Unknown Error", MB_OK | MB_ICONERROR);
    }
    exit(EXIT_FAILURE);
}

LRESULT CALLBACK fnWndProc(HWND, UINT, WPARAM, LPARAM);

std::unique_ptr<Renderer> initApp(UINT width, UINT height, Renderer::CreateInfo& info)
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    THROW_IF_NULL(hInstance);

    WNDCLASSEX wndclass = {};
    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = fnWndProc;
    // We will populate GWLP_USERDATA with the renderer instance pointer.
    wndclass.cbWndExtra = sizeof(void*);
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.lpszClassName = DEMO_NAME;

    RegisterClassEx(&wndclass);

    // GetSystemMetrics is inaccurate, use SystemParametersInfo instead
    RECT workAreaRect = {};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);

    //int workAreaWidth = GetSystemMetrics(SM_CXFULLSCREEN);
    //int workAreaHeight = GetSystemMetrics(SM_CYFULLSCREEN);
    int workAreaWidth = workAreaRect.right - workAreaRect.left;
    int workAreaHeight = workAreaRect.bottom - workAreaRect.top;

    auto dpi = GetDpiForSystem();
    auto factor = dpi / 96.0f;
    SIZE src = // scaled by DPI
    {
        math_utils::round(width * factor),
        math_utils::round(height * factor)
    };
    RECT dst = { 0, 0, workAreaWidth, workAreaHeight };

    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    // Prevent DWM from drawing the window again.
    DWORD dwExStyle = WS_EX_NOREDIRECTIONBITMAP;

    auto wndrect = math_utils::centered(dst, src);
    AdjustWindowRectExForDpi(&wndrect, dwStyle, FALSE, dwExStyle, dpi);

    auto window = CreateWindowEx
    (
        /* dwExStyle    */ dwExStyle,
        /* lpClassName  */ DEMO_NAME,
        /* lpWindowName */ DEMO_NAME,
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
    THROW_IF_NULL(window);

    auto rndr = std::make_unique<Renderer>(window, info);
    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)rndr.get());

    return rndr;
}

LRESULT CALLBACK fnWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (RuntimeError::g_flag)
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    auto rndr = (Renderer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (message)
    {
    case WM_ACTIVATEAPP:
    {
        if (!wParam && rndr->window().fullscreen())
        {
            ShowWindow(rndr->window().ptr, SW_MINIMIZE);
        }
        return 0;
    }
    case WM_PAINT:
    {
        if (rndr)
        {
            rndr->renderNextFrame();
        }
        return 0;
    }
    case WM_SIZE:
    {
        if (rndr && LOWORD(lParam) > 0 && HIWORD(lParam) > 0)
        {
            rndr->onWindowResize();
            rndr->renderNextFrame();
        }
        return 0;
    }
    case WM_SYSKEYDOWN:
    {
        switch (wParam)
        {
        case VK_RETURN:
        {
            auto& window = rndr->window();
            window.setFullscreen(!window.fullscreen());
            break;
        }
        default: break;
        }
        return 0;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_BACK:
        {
            auto& setting = rndr->d3d12DeviceInfo().setting;
            setting.setScaling(!setting.scaling());

            if (setting.scaling())
            {
                auto captionText = std::wstring(DEMO_NAME) + L" (" +
                    std::to_wstring(setting.displayModeIndex()) + L") " +
                    info_utils::text(setting.displayMode());

                SetWindowText(rndr->window().ptr, captionText.c_str());
            }
            else SetWindowText(rndr->window().ptr, DEMO_NAME);

            break;
        }
        case VK_RETURN:
        {
            if (rndr->composition())
            {
                rndr->setComposition(false);

                auto& setting = rndr->d3d12DeviceInfo().setting;
                if (setting.scaling())
                {
                    auto captionText = std::wstring(DEMO_NAME) + L" (" +
                        std::to_wstring(setting.displayModeIndex()) + L") " +
                        info_utils::text(setting.displayMode());

                    SetWindowText(rndr->window().ptr, captionText.c_str());
                }
                else SetWindowText(rndr->window().ptr, DEMO_NAME);
            }
            else // switch the mode
            {
                rndr->setComposition(true);
                SetWindowText(rndr->window().ptr, DEMO_NAME L" (Composition)");
            }
            break;
        }
        case VK_ESCAPE:
        {
            rndr->window().setFullscreen(false);
            break;
        }
        case VK_SPACE:
        {
            auto& property = rndr->d3d12DeviceInfo().property;
            auto displayModeCount = (UINT)property.availableDisplayModes.size();

            auto& setting = rndr->d3d12DeviceInfo().setting;
            setting.setScaling(true);
            setting.setDisplayMode((setting.displayModeIndex() + 1) % displayModeCount);

            auto captionText = std::wstring(DEMO_NAME) + L" (" +
                std::to_wstring(setting.displayModeIndex()) + L") " +
                info_utils::text(setting.displayMode());

            SetWindowText(rndr->window().ptr, captionText.c_str());

            break;
        }
        default: break;
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
