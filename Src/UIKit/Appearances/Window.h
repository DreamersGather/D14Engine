#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/EnumMagic.h"

#include "UIKit/Appearances/Appearance.h"
#include "UIKit/SolidStyle.h"
#include "UIKit/StrokeStyle.h"

namespace d14engine::uikit::appearance
{
    struct Window
    {
        enum class ButtonState
        {
            Idle, Hover, Down, CloseIdle, CloseHover, CloseDown
        };
        struct Appearance : appearance::Appearance
        {
            Appearance();
            static void initialize();

            void changeTheme(WstrParam themeName) override;

            constexpr static auto g_stateCount =
                cpp_lang_utils::enumCount<ButtonState>;

            SolidStyle background = {};
            StrokeStyle stroke = {};

            struct Shadow
            {
                D2D1_COLOR_F color = {};

                float standardDeviation = 3.0f;
            }
            shadow = {};

            struct CaptionPanel
            {
                SolidStyle background = {};
            }
            captionPanel = {};

            struct DecorativeBar
            {
                // Call decorativeBar.loadBrush after changing these.
                D2D1_COLOR_F gradientColors[3] = {};
            }
            decorativeBar = {};

            struct ButtonPanel
            {
                SolidStyle foreground = {};
                SolidStyle background = {};
            }
            buttonPanel[g_stateCount] = {};

            float maskOpacityAboveTabGroup = 0.5f;

            struct ThemeData
            {
                struct Background
                {
                    D2D1_COLOR_F color = {};
                }
                background = {};

                struct Shadow
                {
                    D2D1_COLOR_F color = {};
                }
                shadow = {};

                struct CaptionPanel
                {
                    struct Background
                    {
                        D2D1_COLOR_F color = {};
                    }
                    background = {};
                }
                captionPanel = {};

                struct ButtonPanel
                {
                    SolidStyle foreground = {};
                    SolidStyle background = {};
                }
                buttonPanel[g_stateCount] = {};
            };
            _D14_SET_THEME_DATA_MAP_DECL
        }
        appearanceData = {};
    };
}
