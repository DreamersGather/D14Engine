#include "Common/Precompile.h"

#include "UIKit/ElevatedButton.h"

#include "Common/MathUtils/2D.h"

#include "Renderer/Renderer.h"

#include "UIKit/IconLabel.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    ElevatedButton::ElevatedButton(
        ShrdPtrRefer<IconLabel> content,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        FilledButton(content, roundRadius, rect),
        shadow(size()) { }

    ElevatedButton::ElevatedButton(
        WstrRefer text,
        float roundRadius,
        const D2D1_RECT_F& rect)
        :
        ElevatedButton(
            IconLabel::uniformLayout(text),
            roundRadius,
            rect) { }

    void ElevatedButton::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        FilledButton::onRendererDrawD2d1LayerHelper(rndr);

        shadow.beginDraw(rndr->d2d1DeviceContext());
        {
            auto& shadowSetting = appearance().shadow;

            resource_utils::solidColorBrush()->SetOpacity(1.0f);

            D2D1_ROUNDED_RECT roundedRect =
            {
                math_utils::moveVertex(selfCoordRect(), shadowSetting.offset),
                roundRadiusX, roundRadiusY
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ roundedRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        shadow.endDraw(rndr->d2d1DeviceContext());
    }

    void ElevatedButton::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////
        // Shadow //
        ////////////

        auto& shadowSetting = appearance().shadow;

        shadow.color = shadowSetting.color[(size_t)m_state];
        shadow.standardDeviation = shadowSetting.standardDeviation;

        shadow.configEffectInput(resource_utils::shadowEffect());

        rndr->d2d1DeviceContext()->DrawImage
        (
        /* effect       */ resource_utils::shadowEffect(),
        /* targetOffset */ absolutePosition()
        );

        ////////////
        // Entity //
        ////////////

        FilledButton::onRendererDrawD2d1ObjectHelper(rndr);
    }

    void ElevatedButton::onSizeHelper(SizeEvent& e)
    {
        FilledButton::onSizeHelper(e);

        shadow.loadBitmap(e.size);
    }

    void ElevatedButton::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        FilledButton::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }
}
