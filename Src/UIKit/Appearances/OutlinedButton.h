#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/FlatButton.h"

namespace d14engine::uikit::appearance
{
    struct OutlinedButton
    {
        struct Appearance : AppearanceProxy<FlatButton::Appearance>
        {
            static void initialize();

            void changeTheme(Master& m, WstrParam themeName) override;

            _D14_REF_THEME_DATA_MAP_DECL(FlatButton)
        }
        appearance = {};
    };
}
