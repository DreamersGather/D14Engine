#pragma once

#include "Common/Precompile.h"

#include "Common/MathUtils/Basic.h"

namespace d14engine::uikit::platform_utils
{
    float dpi(); // DPI: Dots Per Inch

    template<typename T>
    T scaledByDpi(T value)
    {
        auto factor = dpi() / 96.0f;
        return math_utils::round<T>((float)value * factor);
    }
    template<typename T>
    T restoredByDpi(T value)
    {
        auto factor = 96.0f / dpi();
        return math_utils::round<T>((float)value * factor);
    }

    SIZE scaledByDpi(const SIZE& sz);
    SIZE restoredByDpi(const SIZE& sz);

    D2D1_SIZE_F scaledByDpi(const D2D1_SIZE_F& sz);
    D2D1_SIZE_F restoredByDpi(const D2D1_SIZE_F& sz);

    POINT scaledByDpi(const POINT& pt);
    POINT restoredByDpi(const POINT& pt);

    D2D1_POINT_2F scaledByDpi(const D2D1_POINT_2F& pt);
    D2D1_POINT_2F restoredByDpi(const D2D1_POINT_2F& pt);

    RECT scaledByDpi(const RECT& rc);
    RECT restoredByDpi(const RECT& rc);

    D2D1_RECT_F scaledByDpi(const D2D1_RECT_F& rc);
    D2D1_RECT_F restoredByDpi(const D2D1_RECT_F& rc);
}
