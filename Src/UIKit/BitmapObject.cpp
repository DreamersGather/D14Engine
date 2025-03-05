#include "Common/Precompile.h"

#include "UIKit/BitmapObject.h"

namespace d14engine::uikit
{
    BitmapObject::BitmapObject(ComPtrParam<ID2D1Bitmap1> data) : data(data) {}

    bool BitmapObject::empty() const
    {
        if (data != nullptr)
        {
            auto pixSize = data->GetPixelSize();
            return pixSize.width == 0 || pixSize.height == 0;
        }
        else return true;
    }

    D2D1_INTERPOLATION_MODE BitmapObject::g_interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;

    D2D1_INTERPOLATION_MODE BitmapObject::getInterpolationMode() const
    {
        D2D1_INTERPOLATION_MODE mode = {};
        if (interpolationMode.has_value())
        {
            mode = interpolationMode.value();
        }
        else mode = g_interpolationMode;

        // This mode requires a non-empty bitmap.
        if (mode == D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC && empty())
        {
            return D2D1_INTERPOLATION_MODE_LINEAR;
        }
        else return mode;
    }
}
