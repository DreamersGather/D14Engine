#include "Common/Precompile.h"

#include "UIKit/HorzSlider.h"

namespace d14engine::uikit
{
    HorzSlider::HorzSlider(
        const D2D1_RECT_F& rect,
        float value,
        float minValue,
        float maxValue)
        :
        Slider(rect, value, minValue, maxValue)
    {
        ///////////////////////////
        // Load Cached Resources //
        ///////////////////////////

        valueLabelRes.loadShadowMask = [this]
        {
            auto& setting = appearance().valueLabel;

            auto& rectSize = setting.mainRect.geometry.size;
            auto& trngSize = setting.sideTriangle.size;

            valueLabelRes.shadowMask.loadBitmap(
                rectSize.width, rectSize.height + trngSize.height);
        };
        valueLabelRes.loadShadowMask();
    }

    void HorzSlider::onSizeHelper(SizeEvent& e)
    {
        Slider::onSizeHelper(e);

        /////////////////////////////
        // Reload Cached Resources //
        /////////////////////////////

        valueLabelRes.loadShadowMask();
    }

    D2D1_POINT_2F HorzSlider::valueToOffset(float value) const
    {
        float valueSpan = m_maxValue - m_minValue;

        auto w = width();
        if (w > 0.0f && valueSpan > 0.0f)
        {
            if (value <= m_minValue) return { 0.0f, 0.0f };
            else if (value >= m_maxValue) return { w, 0.0f };
            else return { (value - m_minValue) / valueSpan * w, 0.0f };
        }
        else return { 0.0f, 0.0f };
    }

    float HorzSlider::offsetToValue(const D2D1_POINT_2F& offset) const
    {
        float valueSpan = m_maxValue - m_minValue;

        auto w = width();
        if (w > 0.0f && valueSpan > 0.0f)
        {
            if (offset.x <= 0.0f) return m_minValue;
            else if (offset.x >= w) return m_maxValue;
            else return m_minValue + (offset.x / w) * valueSpan;
        }
        else return 0.0f;
    }

    D2D1_RECT_F HorzSlider::thumbAreaRect(const D2D1_RECT_F& flatRect) const
    {
        return math_utils::stretch(flatRect, { appearance().thumbAreaExtension, 0.0f });
    }

    D2D1_RECT_F HorzSlider::valueLabelMainRectInShadow() const
    {
        auto& setting = appearance().valueLabel;

        auto& rectSize = setting.mainRect.geometry.size;

        return math_utils::rect({ 0.0f, 0.0f }, rectSize);
    }

    math_utils::Triangle2D HorzSlider::valueLabelSideTriangleInShadow() const
    {
        auto& setting = appearance().valueLabel;

        auto& rectSize = setting.mainRect.geometry.size;
        auto& trngSize = setting.sideTriangle.size;

        float halfRectWidth = rectSize.width * 0.5f;
        float halfTrngWidth = trngSize.width * 0.5f;

        D2D1_POINT_2F vertex0 =
        {
            halfRectWidth,
            rectSize.height + trngSize.height
        };
        D2D1_POINT_2F vertex1 =
        {
            halfRectWidth - halfTrngWidth,
            rectSize.height
        };
        D2D1_POINT_2F vertex2 =
        {
            halfRectWidth + halfTrngWidth,
            rectSize.height
        };
        return { vertex0, vertex1, vertex2 };
    }

    D2D1_RECT_F HorzSlider::filledBarAbsoluteRect() const
    {
        float barHeight = appearance().bar.filled.geometry.height;
        float barRectTop = m_absoluteRect.top + (height() - barHeight) * 0.5f;
        return
        {
            m_absoluteRect.left,
            barRectTop,
            m_absoluteRect.left + valueToOffset(m_value).x,
            barRectTop + barHeight
        };
    }

    D2D1_RECT_F HorzSlider::completeBarAbsoluteRect() const
    {
        float barHeight = appearance().bar.complete.geometry.height;
        float barRectTop = m_absoluteRect.top + (height() - barHeight) * 0.5f;
        return
        {
            m_absoluteRect.left,
            barRectTop,
            m_absoluteRect.right,
            barRectTop + barHeight
        };
    }

    D2D1_RECT_F HorzSlider::handleAbsoluteRect() const
    {
        auto& handleSize = appearance().handle.geometry.size;
        return math_utils::rect
        (
            m_absoluteRect.left + valueToOffset(m_value).x - handleSize.width * 0.5f,
            m_absoluteRect.top + (height() - handleSize.height) * 0.5f,
            handleSize.width,
            handleSize.height
        );
    }

    D2D1_RECT_F HorzSlider::valueLabelSelfCoordRect() const
    {
        auto& setting = appearance().valueLabel;

        auto& rectSize = setting.mainRect.geometry.size;
        auto& trngSize = setting.sideTriangle.size;

        D2D1_RECT_F rect = {};

        rect.left = valueToOffset(m_value).x - rectSize.width * 0.5f;
        rect.right = rect.left + rectSize.width;
        rect.bottom = height() * 0.5f - setting.offset - trngSize.height;
        rect.top = rect.bottom - rectSize.height;

        return rect;
    }

    D2D1_RECT_F HorzSlider::valueLabelShadowSelfCoordRect() const
    {
        auto& trngSize = appearance().valueLabel.sideTriangle.size;

        return math_utils::increaseBottom(valueLabelSelfCoordRect(), trngSize.height);
    }
}
