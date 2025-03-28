﻿#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/Label.h"

namespace d14engine::uikit::appearance
{
    struct RawTextInput
    {
        struct Appearance : AppearanceProxy<Label::Appearance>
        {
            static void initialize();

            void changeTheme(Master& m, WstrRefer themeName) override;

            struct BottomLine
            {
                float bottomOffset = -0.5f;

                SolidStyle background = {};

                float strokeWidth = 1.0f;
            }
            bottomLine = {};

            struct ThemeData
            {
                SolidStyle background = {};
                StrokeStyle stroke = {};

                struct BottomLine
                {
                    SolidStyle background = {};
                }
                bottomLine = {};
            };
            _D14_SET_THEME_DATA_MAP_DECL
        }
        appearanceData = {};
    };
}
