#pragma once

#include "Common/Precompile.h"

#include "UIKit/ColorUtils.h"
#include "UIKit/Window.h"

namespace d14engine::uikit
{
    struct MainWindow : Window
    {
        MainWindow(
            ShrdPtrParam<IconLabel> caption,
            float captionPanelHeight = 32.0f,
            float decorativeBarHeight = 4.0f);

        MainWindow(
            WstrParam title = L"Untitled",
            float captionPanelHeight = 32.0f,
            float decorativeBarHeight = 4.0f);

        void onInitializeFinish() override;

    protected:
        // Helps set the correct state after restored from minimized.
        DisplayState m_originalDisplayState = {};

        /////////////////////////
        // Immersive Dark Mode //
        /////////////////////////

    protected:
        // This value is supported starting with Windows 11 Build 22000!
        bool m_immersiveDarkMode = false;

    public:
        bool immersiveDarkMode() const;
        // Returns whether the setting was successful.
        bool setImmersiveDarkMode(bool value);

        //////////////////
        // Corner State //
        //////////////////

    public:
        using CornerState = DWM_WINDOW_CORNER_PREFERENCE;

        constexpr static CornerState Round = DWMWCP_ROUND;
        constexpr static CornerState RoundSmall = DWMWCP_ROUNDSMALL;
        constexpr static CornerState DoNotRound = DWMWCP_DONOTROUND;

    protected:
        // This value is supported starting with Windows 11 Build 22000!
        CornerState m_cornerState = {};

        // Helps set the correct state after restored from maximized.
        CornerState m_originalCornerState = {};

    public:
        CornerState cornerState() const;
        // Returns whether the setting was successful.
        bool setCornerState(CornerState state);

        //////////////////
        // Border Color //
        //////////////////

    public:
        using BorderColor = color_utils::HEX;

        constexpr static BorderColor NoneColor = DWMWA_COLOR_NONE;
        constexpr static BorderColor DefaultColor = DWMWA_COLOR_DEFAULT;

    protected:
        // This value is supported starting with Windows 11 Build 22000!
        BorderColor m_borderColor = {};

        bool setBorderColorAttr(BorderColor color);

        bool m_accentBorder = false;

    public:
        BorderColor borderColor() const;
        // Returns whether the setting was successful.
        bool setBorderColor(BorderColor color);

        bool accentBorder() const;
        bool setAccentBorder(bool value);

    protected:
        // Panel
        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        // Window
        void onCloseHelper() override;

        void onRestoreHelper() override;

        void onMinimizeHelper() override;

        void onMaximizeHelper() override;
    };
}
