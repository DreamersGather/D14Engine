#include "Common/Precompile.h"

#include "UIKit/FlatButton.h"

#include "UIKit/IconLabel.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    FlatButton::FlatButton(
        ShrdPtrRefer<IconLabel> content,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        Button(content, roundRadius, rect) { }

    FlatButton::FlatButton(
        WstrRefer text,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        FlatButton(
            IconLabel::uniformLayout(text),
            roundRadius,
            rect) { }

    void FlatButton::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        auto& dstSetting = Button::appearance();
        auto& srcSetting = appearance().main[(size_t)m_state];

        dstSetting.foreground = srcSetting.foreground;
        dstSetting.background = srcSetting.background;
        dstSetting.stroke = srcSetting.stroke;

        Button::onRendererDrawD2d1ObjectHelper(rndr);
    }

    void FlatButton::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Button::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }
}
