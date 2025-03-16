#pragma once

#include "Common/Precompile.h"

#include "UIKit/Layout.h"

namespace d14engine::uikit
{
    struct ConstraintLayoutGeometryInfo
    {
        bool keepWidth = true;

        struct HorzDistance
        {
            Optional<float> ToLeft = {}, ToRight = {};
        }
        Left = {}, Right = {};

        bool keepHeight = true;

        struct VertDistance
        {
            Optional<float> ToTop = {}, ToBottom = {};
        }
        Top = {}, Bottom = {};
    };

    struct ConstraintLayout : Layout<ConstraintLayoutGeometryInfo>
    {
        explicit ConstraintLayout(const D2D1_RECT_F& rect = {});

    public:
        using Layout::updateElement;

    protected:
        void updateElement(ShrdPtrRefer<Panel> elem, const GeometryInfo& geoInfo) override;
    };
}
