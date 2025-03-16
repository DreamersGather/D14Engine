#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/OutlinedButton.h"
#include "UIKit/FlatButton.h"

namespace d14engine::uikit
{
    struct OutlinedButton : appearance::OutlinedButton, FlatButton
    {
        OutlinedButton(
            ShrdPtrRefer<IconLabel> content,
            float roundRadius = 0.0f,
            const D2D1_RECT_F& rect = {});

        OutlinedButton(
            WstrRefer text = L"Button",
            float roundRadius = 0.0f,
            const D2D1_RECT_F& rect = {});

        _D14_SET_APPEARANCE_PROPERTY(OutlinedButton)

    protected:
        // Panel
        void onChangeThemeStyleHelper(const ThemeStyle& style) override;
    };
}
