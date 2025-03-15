#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/EnumMagic.h"

#include "UIKit/Appearances/Appearance.h"
#include "UIKit/SolidStyle.h"
#include "UIKit/StrokeStyle.h"

namespace d14engine::uikit::appearance
{
    struct ViewItem
    {
        enum class State
        {
            Idle, Hover, Selected, FocusSelected, FocusSelectedHover
        };
        struct Appearance : appearance::Appearance
        {
            static void initialize();

            void changeTheme(WstrParam themeName) override;

            constexpr static auto g_stateCount =
                cpp_lang_utils::enumCount<State>;

            struct Main
            {
                SolidStyle background = {}; StrokeStyle stroke = {};
            }
            main[g_stateCount] = {};

            struct ThemeData { Main main[g_stateCount] = {}; };

            _D14_SET_THEME_DATA_MAP_DECL
        }
        appearanceData = {};
    };
}
