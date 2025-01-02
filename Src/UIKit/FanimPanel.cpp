#include "Common/Precompile.h"

#include "UIKit/FanimPanel.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    void FanimPanel::onRendererUpdateObject2DHelper(Renderer* rndr)
    {
        if (m_isPlayAnimation) bitmapData.update(rndr);
    }

    void FanimPanel::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        bitmapData.draw(rndr, m_absoluteRect);
    }
}
