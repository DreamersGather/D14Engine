#include "Common/Precompile.h"

#include "UIKit/MaskObject.h"

#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"

#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/PlatformUtils.h"

namespace d14engine::uikit
{
    MaskObject::MaskObject(const D2D1_SIZE_F& size)
    {
        loadBitmap(size);
    }

    MaskObject::MaskObject(float width, float height)
    {
        loadBitmap(width, height);
    }

    void MaskObject::loadBitmap(const D2D1_SIZE_F& size)
    {
        THROW_IF_NULL(Application::g_app);

        auto rndr = Application::g_app->renderer();

        auto pixSize = math_utils::roundu(
            platform_utils::scaledByDpi(size));

        data = bitmap_utils::loadBitmap(
            pixSize.width, pixSize.height,
            nullptr, D2D1_BITMAP_OPTIONS_TARGET);
    }

    void MaskObject::loadBitmap(float width, float height)
    {
        loadBitmap({ width, height });
    }

    void MaskObject::beginDraw(ID2D1DeviceContext* context, const D2D1_MATRIX_3X2_F& transform)
    {
        // It is recommended to call SetTarget before BeginDraw.
        // The program may crash if the previous target is a synchronized
        // resource and still bound to the context when calling BeginDraw.
        context->SetTarget(data.Get());
        context->BeginDraw();
        context->SetTransform(transform);
        context->Clear(color);
    }

    void MaskObject::endDraw(ID2D1DeviceContext* context)
    {
        THROW_IF_FAILED(context->EndDraw());
        // We do not reset the target here since in beginDraw
        // SetTarget is called before BeginDraw, so the target is always valid
        // when BeginDraw called, otherwise the program may crash unexpectedly.
    }
}
