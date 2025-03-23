#include "Common/Precompile.h"

#include "UIKit/GridLayout.h"

namespace d14engine::uikit
{
    GridLayout::GridLayout(
        size_t horzCellCount,
        size_t vertCellCount,
        float horzMargin,
        float horzSpacing,
        float vertMargin,
        float vertSpacing,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        Layout(rect),
        m_horzCellCount(horzCellCount),
        m_vertCellCount(vertCellCount),
        m_horzMargin(horzMargin),
        m_horzSpacing(horzSpacing),
        m_vertMargin(vertMargin),
        m_vertSpacing(vertSpacing) { }

    void GridLayout::updateCellDeltaInfo()
    {
        m_horzCellDelta = (width() - 2.0f * m_horzMargin) / m_horzCellCount;
        m_vertCellDelta = (height() - 2.0f * m_vertMargin) / m_vertCellCount;
    }

    float GridLayout::horzCellDelta() const
    {
        return m_horzCellDelta;
    }

    size_t GridLayout::horzCellCount() const
    {
        return m_horzCellCount;
    }

    float GridLayout::vertCellDelta() const
    {
        return m_vertCellDelta;
    }

    size_t GridLayout::vertCellCount() const
    {
        return m_vertCellCount;
    }

    void GridLayout::setCellCount(size_t horz, size_t vert)
    {
        m_horzCellCount = std::max(horz, 1_uz);
        m_vertCellCount = std::max(vert, 1_uz);

        updateCellDeltaInfo();
        updateAllElements();
    }

    float GridLayout::horzMargin() const
    {
        return m_horzMargin;
    }

    float GridLayout::horzSpacing() const
    {
        return m_horzSpacing;
    }

    float GridLayout::vertMargin() const
    {
        return m_vertMargin;
    }

    float GridLayout::vertSpacing() const
    {
        return m_vertSpacing;
    }

    void GridLayout::setMargin(float horz, float vert)
    {
        m_horzMargin = horz;
        m_vertMargin = vert;

        updateCellDeltaInfo();
        updateAllElements();
    }

    void GridLayout::setSpacing(float horz, float vert)
    {
        m_horzSpacing = horz;
        m_vertSpacing = vert;

        updateAllElements();
    }

    void GridLayout::updateElement(ShrdPtrRefer<Panel> elem, const GeometryInfo& geoInfo)
    {
        if (geoInfo.isFixedSize)
        {
            float width = geoInfo.axis.x.count * m_horzCellDelta;
            float height = geoInfo.axis.y.count * m_vertCellDelta;

            float offsetX = m_horzMargin + (width - elem->width()) * 0.5f;
            float offsetY = m_vertMargin + (height - elem->height()) * 0.5f;

            elem->setPosition
            (
                std::round(geoInfo.axis.x.offset * m_horzCellDelta + offsetX),
                std::round(geoInfo.axis.y.offset * m_vertCellDelta + offsetY)
            );
        }
        else // variable size
        {
            float offsetX = m_horzMargin + m_horzSpacing + geoInfo.spacing.left;
            float offsetY = m_vertMargin + m_vertSpacing + geoInfo.spacing.top;

            float horzSpacing = 2.0f * m_horzSpacing + geoInfo.spacing.left + geoInfo.spacing.right;
            float vertSpacing = 2.0f * m_vertSpacing + geoInfo.spacing.top + geoInfo.spacing.bottom;

            elem->transform
            (
                std::round(geoInfo.axis.x.offset * m_horzCellDelta + offsetX),
                std::round(geoInfo.axis.y.offset * m_vertCellDelta + offsetY),
                std::round(geoInfo.axis.x.count * m_horzCellDelta - horzSpacing),
                std::round(geoInfo.axis.y.count * m_vertCellDelta - vertSpacing)
            );
        }
    }

    void GridLayout::onSizeHelper(SizeEvent& e)
    {
        // We must call updateCellDeltaInfo before Layout::onSizeHelper
        // as updateAllElements depends on the updated geometry information.
        updateCellDeltaInfo();

        Layout::onSizeHelper(e);
    }
}
