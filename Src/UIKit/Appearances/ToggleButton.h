#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/FlatButton.h"

namespace d14engine::uikit::appearance
{
    struct ToggleButton
    {
        struct Appearance : FlatButton::Appearance
        {
            static void initialize();

            void changeTheme(WstrParam themeName) override;

            static ThemeDataMap g_themeData;
        }
        appearance = {};
    };
}
