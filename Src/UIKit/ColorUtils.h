#pragma once

#include "Common/Precompile.h"

#include "Common/MathUtils/Basic.h"

namespace d14engine::uikit::color_utils
{
    //////////
    // ABGR //
    //////////

    struct ABGR
    {
        UINT32 data = {};

        //------------------------------------------------------------------
        // Constructor
        //------------------------------------------------------------------

        ABGR() = default;

        explicit constexpr ABGR(const D2D1_COLOR_F& cf)
            :
            data
            (
                std::clamp(math_utils::round(cf.a * 255.0f), 0, 255) << 24 |
                std::clamp(math_utils::round(cf.b * 255.0f), 0, 255) << 16 |
                std::clamp(math_utils::round(cf.g * 255.0f), 0, 255) << 8  |
                std::clamp(math_utils::round(cf.r * 255.0f), 0, 255) << 0
            )
        {
            // Here left blank intentionally.
        }
        constexpr ABGR(UINT32 abgr) : data(abgr) {}

        //------------------------------------------------------------------
        // Conversion
        //------------------------------------------------------------------

        explicit constexpr operator D2D1_COLOR_F() const
        {
            return
            {
                std::clamp((float)((data & 0x00'00'00'ff) >>  0) / 255.0f, 0.0f, 1.0f),
                std::clamp((float)((data & 0x00'00'ff'00) >>  8) / 255.0f, 0.0f, 1.0f),
                std::clamp((float)((data & 0x00'ff'00'00) >> 16) / 255.0f, 0.0f, 1.0f),
                std::clamp((float)((data & 0xff'00'00'00) >> 24) / 255.0f, 0.0f, 1.0f)
            };
        }

        //------------------------------------------------------------------
        // Comparison
        //------------------------------------------------------------------

        constexpr bool operator==(const ABGR& other) const
        {
            return data == other.data;
        }
    };
}
