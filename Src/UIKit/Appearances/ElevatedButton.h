﻿#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/EnumMagic.h"

#include "UIKit/Appearances/Appearance.h"
#include "UIKit/Button.h"

namespace d14engine::uikit::appearance
{
    struct ElevatedButton
    {
        struct Appearance : appearance::Appearance
        {
            static void initialize();

            void changeTheme(WstrParam themeName) override;

            constexpr static auto g_stateCount =
                cpp_lang_utils::enumCount<uikit::Button::State>;

            struct Shadow
            {
                // Call loadShadowBitmap after changing this field.
                D2D1_RECT_F offset = { 2.0f, 3.0f, -2.0f, 0.0f };

                D2D1_COLOR_F color[g_stateCount] = {};

                float standardDeviation = 3.0f;
            }
            shadow = {};

            struct ThemeData
            {
                struct Shadow
                {
                    D2D1_COLOR_F color[g_stateCount] = {};
                }
                shadow = {};
            };
            _D14_SET_THEME_DATA_MAP_DECL
        }
        appearance = {};
    };
}
