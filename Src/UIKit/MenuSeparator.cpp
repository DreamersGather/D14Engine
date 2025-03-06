#include "Common/Precompile.h"

#include "UIKit/MenuSeparator.h"

#include "Common/MathUtils/2D.h"

#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    MenuSeparator::MenuSeparator(const D2D1_RECT_F& rect)
        :
        MenuItem((ShrdPtrParam<Panel>)nullptr, rect) { isTriggerItem = false; }


    void MenuSeparator::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        auto& bkgn = getAppearance().background;

        resource_utils::g_solidColorBrush->SetColor(bkgn.color);
        resource_utils::g_solidColorBrush->SetOpacity(bkgn.opacity);

        auto& bkgnRect = m_absoluteRect;

        rndr->d2d1DeviceContext()->DrawLine
        (
        /* point0      */ math_utils::leftCenter(bkgnRect),
        /* point1      */ math_utils::rightCenter(bkgnRect),
        /* brush       */ resource_utils::g_solidColorBrush.Get(),
        /* strokeWidth */ getAppearance().strokeWidth
        );
    }

    void MenuSeparator::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        MenuItem::onChangeThemeStyleHelper(style);

        getAppearance().changeTheme(ViewItem::getAppearance(), style.name);
    }
}
