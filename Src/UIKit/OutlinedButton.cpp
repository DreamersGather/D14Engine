#include "Common/Precompile.h"

#include "UIKit/OutlinedButton.h"

#include "UIKit/IconLabel.h"
#include "UIKit/ResourceUtils.h"

namespace d14engine::uikit
{
    OutlinedButton::OutlinedButton(
        ShrdPtrRefer<IconLabel> content,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        FlatButton(content, roundRadius, rect) { }

    OutlinedButton::OutlinedButton(
        WstrRefer text,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        OutlinedButton(
            IconLabel::uniformLayout(text),
            roundRadius,
            rect) { }

    void OutlinedButton::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Button::onChangeThemeStyleHelper(style);

        appearance().changeTheme(FlatButton::appearance(), style.name);
    }
}
