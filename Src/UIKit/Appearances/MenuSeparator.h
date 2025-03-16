#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/ViewItem.h"

namespace d14engine::uikit::appearance
{
    struct MenuSeparator
    {
        struct Appearance : AppearanceProxy<ViewItem::Appearance>
        {
            static void initialize();

            void changeTheme(Master& m, WstrRefer themeName) override;

            SolidStyle background = {};

            float strokeWidth = 1.0f;

            struct ThemeData
            {
                SolidStyle background = {};
            };
            _D14_SET_THEME_DATA_MAP_DECL
        }
        appearanceData = {};
    };
}
