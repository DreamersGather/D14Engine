#include "Common/Precompile.h"

#include "UIKit/MainWindow.h"

#include "Common/MathUtils/GDI.h"
#include "Common/RuntimeError.h"

#include "UIKit/Application.h"
#include "UIKit/IconLabel.h"
#include "UIKit/PlatformUtils.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    MainWindow::MainWindow(
        ShrdPtrRefer<IconLabel> caption,
        float captionPanelHeight,
        float decorativeBarHeight)
        :
        Panel({}, resource_utils::solidColorBrush()),
        Window(caption, {}, captionPanelHeight, decorativeBarHeight)
    {
        THROW_IF_NULL(Application::g_app);

        drawBufferRes.shadow().enabled = false;

        // Try to adapt the fluent design of Windows 11.
        if (!setCornerState(Round) || !setBorderColor(DefaultColor))
        {
            // Fall back to the accent border for Windows 10.
            appearance().stroke.opacity = 1.0f;
        }
        draggingTarget = DraggingTarget::RootWindow;

        auto& callback = Application::g_app->win32WindowSettings.callback;
        callback.f_onClientAreaSize = [this]
        (const SIZE& pixSize)
        {
            auto dipSize = platform_utils::restoredByDpi(pixSize);
            setSize((float)dipSize.cx, (float)dipSize.cy);
        };
        callback.f_onRestoreFromMinimized = [this]
        {
            m_displayState = m_originalDisplayState;
        };
        respondSetForegroundEvent = false; // bottom-most
    }

    MainWindow::MainWindow(
        WstrRefer title,
        float captionPanelHeight,
        float decorativeBarHeight)
        :
        MainWindow(
            IconLabel::labelExpandedLayout(title),
            captionPanelHeight,
            decorativeBarHeight) { }

    void MainWindow::onInitializeFinish()
    {
        Window::onInitializeFinish();

        THROW_IF_NULL(Application::g_app);

        RECT wndrect = {};
        GetClientRect(Application::g_app->win32Window(), &wndrect);

        auto pixSize = math_utils::size(wndrect);
        auto dipSize = platform_utils::restoredByDpi(pixSize);

        setSize((float)dipSize.cx, (float)dipSize.cy);
    }

    MainWindow::CornerState MainWindow::cornerState() const
    {
        return m_cornerState;
    }

    bool MainWindow::setCornerState(CornerState state)
    {
        THROW_IF_NULL(Application::g_app);

        auto window = Application::g_app->win32Window();

        auto ret = SUCCEEDED(DwmSetWindowAttribute
        (
        /* hwnd        */ window,
        /* dwAttribute */ DWMWA_WINDOW_CORNER_PREFERENCE,
        /* pvAttribute */ &state,
        /* cbAttribute */ sizeof(state))
        );
        if (ret)
        {
            m_cornerState = state;
            if (m_displayState != Normal)
            {
                m_originalCornerState = state;
            }
        }
        return ret;
    }

    bool MainWindow::setBorderColorAttr(BorderColor color)
    {
        THROW_IF_NULL(Application::g_app);

        auto window = Application::g_app->win32Window();

        auto ret = SUCCEEDED(DwmSetWindowAttribute
        (
        /* hwnd        */ window,
        /* dwAttribute */ DWMWA_BORDER_COLOR,
        /* pvAttribute */ &color,
        /* cbAttribute */ sizeof(color))
        );
        if (ret) m_borderColor = color;
        return ret;
    }

    MainWindow::BorderColor MainWindow::borderColor() const
    {
        return m_borderColor;
    }

    bool MainWindow::setBorderColor(BorderColor color)
    {
        THROW_IF_NULL(Application::g_app);

        if (m_accentBorder && color != NoneColor)
        {
            return false;
        }
        auto window = Application::g_app->win32Window();

        return setBorderColorAttr(color);
    }

    bool MainWindow::accentBorder() const
    {
        return m_accentBorder;
    }

    bool MainWindow::setAccentBorder(bool value)
    {
        THROW_IF_NULL(Application::g_app);

        bool ret = false;
        if (m_borderColor != NoneColor)
        {
            if (value)
            {
                auto& color = Application::g_app->themeStyle().color;
                ret = setBorderColorAttr((BorderColor)color);
            }
            else ret = setBorderColorAttr(m_borderColor);
        }
        if (ret) m_accentBorder = value;
        return ret;
    }

    void MainWindow::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Window::onChangeThemeStyleHelper(style);

        if (m_accentBorder && m_borderColor != NoneColor)
        {
            setBorderColorAttr((BorderColor)style.color);
        }
    }

    void MainWindow::onCloseHelper()
    {
        Window::onCloseHelper();

        THROW_IF_NULL(Application::g_app);

        Application::g_app->exit();
    }

    void MainWindow::onRestoreHelper()
    {
        Window::onRestoreHelper();

        THROW_IF_NULL(Application::g_app);

        setCornerState(m_originalCornerState);

        isDraggable = true;
        m_originalDisplayState = Normal;

        auto hwnd = Application::g_app->win32Window();

        RECT wndrect = {};
        GetClientRect(hwnd, &wndrect);

        auto pixSize = math_utils::size(wndrect);
        auto dipSize = platform_utils::restoredByDpi(pixSize);

        setSize((float)dipSize.cx, (float)dipSize.cy);

        ShowWindow(hwnd, SW_NORMAL);
    }

    void MainWindow::onMinimizeHelper()
    {
        Window::onMinimizeHelper();

        THROW_IF_NULL(Application::g_app);

        ShowWindow(Application::g_app->win32Window(), SW_MINIMIZE);
    }

    void MainWindow::onMaximizeHelper()
    {
        Window::onMaximizeHelper();

        THROW_IF_NULL(Application::g_app);

        m_originalCornerState = m_cornerState;
        setCornerState(DoNotRound);

        isDraggable = false;
        m_originalDisplayState = Maximized;

        auto hwnd = Application::g_app->win32Window();

        RECT wndrect = {};
        GetClientRect(hwnd, &wndrect);

        auto pixSize = math_utils::size(wndrect);
        auto dipSize = platform_utils::restoredByDpi(pixSize);

        setSize((float)dipSize.cx, (float)dipSize.cy);

        ShowWindow(hwnd, SW_MAXIMIZE);
    }
}
