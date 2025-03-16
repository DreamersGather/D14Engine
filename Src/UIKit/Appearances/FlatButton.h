#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/EnumMagic.h"

#include "UIKit/Appearances/Appearance.h"
#include "UIKit/Button.h"

namespace d14engine::uikit::appearance
{
    struct FlatButton
    {
        struct Appearance : appearance::Appearance
        {
            static void initialize();

            void changeTheme(WstrRefer themeName) override;

            constexpr static auto g_stateCount =
                cpp_lang_utils::enumCount<uikit::Button::State>;

            struct Main
            {
                SolidStyle foreground = {};
                SolidStyle background = {};
                StrokeStyle stroke = {};
            }
            main[g_stateCount] = {};

            struct ThemeData
            {
                Main main[g_stateCount] = {};
            };
            _D14_SET_THEME_DATA_MAP_DECL
        }
        appearanceData = {};
    };
}
