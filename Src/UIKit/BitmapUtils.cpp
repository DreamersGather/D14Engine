#include "Common/Precompile.h"

#include "UIKit/BitmapUtils.h"

#include "Common/DirectXError.h"
#include "Common/ResourcePack.h"

#include "Renderer/GraphUtils/Bitmap.h"
#include "Renderer/Renderer.h"

#include "UIKit/Application.h"
#include "UIKit/PlatformUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit::bitmap_utils
{
    void saveBitmap(ID2D1Bitmap1* bitmap, WstrParam imagePath, const GUID& format)
    {
        THROW_IF_NULL(Application::g_app);

        auto factory = graph_utils::bitmap::factory();

        /////////////////////////////
        // Create file and stream. //
        /////////////////////////////

        ComPtr<IWICStream> stream = {};
        THROW_IF_FAILED(factory->CreateStream(&stream));

        THROW_IF_FAILED(stream->InitializeFromFilename
        (
        /* wzFileName      */ imagePath.c_str(),
        /* dwDesiredAccess */ GENERIC_WRITE
        ));

        /////////////////////////////
        // Export image to stream. //
        /////////////////////////////

        ComPtr<IWICBitmapEncoder> bitmapEncoder = {};

        THROW_IF_FAILED(factory->CreateEncoder
        (
        /* guidContainerFormat */ format,
        /* pguidVendor         */ nullptr,
        /* ppIEncoder          */ &bitmapEncoder
        ));
        THROW_IF_FAILED(bitmapEncoder->Initialize
        (
        /* pIStream    */ stream.Get(),
        /* cacheOption */ WICBitmapEncoderNoCache
        ));
        ComPtr<IWICBitmapFrameEncode> frameEncode = {};

        THROW_IF_FAILED(bitmapEncoder->CreateNewFrame
        (
        /* ppIFrameEncode    */ &frameEncode,
        /* ppIEncoderOptions */ nullptr
        ));
        THROW_IF_FAILED(frameEncode->Initialize(nullptr));

        ComPtr<IWICImageEncoder> imageEncoder = {};

        THROW_IF_FAILED(factory->CreateImageEncoder
        (
        /* pD2DDevice        */ Application::g_app->dx12Renderer()->d2d1Device(),
        /* ppWICImageEncoder */ &imageEncoder
        ));
        auto pixSize = bitmap->GetPixelSize();
        WICImageParameters imageParams =
        {
            .PixelFormat = bitmap->GetPixelFormat(),
            .Top         = 0.0f,
            .Left        = 0.0f,
            .PixelWidth  = pixSize.width,
            .PixelHeight = pixSize.height
        };
        bitmap->GetDpi(&imageParams.DpiX, &imageParams.DpiY);

        THROW_IF_FAILED(imageEncoder->WriteFrame
        (
        /* pImage           */ bitmap,
        /* pFrameEncode     */ frameEncode.Get(),
        /* pImageParameters */ &imageParams
        ));
        THROW_IF_FAILED(frameEncode->Commit());
        THROW_IF_FAILED(bitmapEncoder->Commit());
        THROW_IF_FAILED(stream->Commit(STGC_DEFAULT));
    }

    ComPtr<ID2D1Bitmap1> loadBitmap(UINT width, UINT height, BYTE* data, D2D1_BITMAP_OPTIONS options)
    {
        THROW_IF_NULL(Application::g_app);

        auto dpi = platform_utils::dpi();

        D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1
        (
        /* bitmapOptions */ options,
        /* pixelFormat   */ D2D1::PixelFormat(Renderer::g_renderTargetFormat, D2D1_ALPHA_MODE_PREMULTIPLIED),
        /* dpiX          */ dpi,
        /* dpiY          */ dpi
        );
        ComPtr<ID2D1Bitmap1> bitmap = {};
        // Here we assume the pixel format is R8G8B8A8/B8G8R8A8 for simplicity,
        // so the pitch (byte count of each scanline) is set to (4 * width).
        THROW_IF_FAILED(Application::g_app->dx12Renderer()->d2d1DeviceContext()->CreateBitmap
        (
        /* size             */ { width, height },
        /* sourceData       */ data,
        /* pitch            */ 4 * width,
        /* bitmapProperties */ props,
        /* bitmap           */ &bitmap
        ));
        return bitmap;
    }

    ComPtr<ID2D1Bitmap1> loadBitmap(WstrParam imagePath, D2D1_BITMAP_OPTIONS options)
    {
        THROW_IF_NULL(Application::g_app);

        auto source = graph_utils::bitmap::load(imagePath);

        auto dpi = platform_utils::dpi();

        D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1
        (
        /* bitmapOptions */ options,
        /* pixelFormat   */ D2D1::PixelFormat(Renderer::g_renderTargetFormat, D2D1_ALPHA_MODE_PREMULTIPLIED),
        /* dpiX          */ dpi,
        /* dpiY          */ dpi
        );
        ComPtr<ID2D1Bitmap1> bitmap = {};
        THROW_IF_FAILED(Application::g_app->dx12Renderer()->d2d1DeviceContext()->CreateBitmapFromWicBitmap
        (
        /* wicBitmapSource  */ source.Get(),
        /* bitmapProperties */ props,
        /* bitmap           */ &bitmap
        ));
        return bitmap;
    }

    ComPtr<ID2D1Bitmap1> loadPackedBitmap(WstrParam resName, WstrParam resType, D2D1_BITMAP_OPTIONS options)
    {
        THROW_IF_NULL(Application::g_app);

        ////////////////////////////////
        // Create stream from memory. //
        ////////////////////////////////

        auto src = loadResource(resName, resType);

        auto mem = GlobalAlloc(GMEM_MOVEABLE, src.size);
        THROW_IF_NULL(mem);
        auto dst = GlobalLock(mem);
        THROW_IF_NULL(dst);

        // TODO: Can we avoid a copy here?
        // (create a stream object directly with the memory pointer)
        memcpy(dst, src.data, src.size);

        GlobalUnlock(mem);
        GlobalFree(mem);

        ComPtr<IStream> stream = {};
        THROW_IF_FAILED(CreateStreamOnHGlobal(mem, FALSE, &stream));

        ////////////////////////////////
        // Create bitmap from stream. //
        ////////////////////////////////

        auto source = graph_utils::bitmap::load(stream.Get());

        auto dpi = platform_utils::dpi();

        D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1
        (
        /* bitmapOptions */ options,
        /* pixelFormat   */ D2D1::PixelFormat(Renderer::g_renderTargetFormat, D2D1_ALPHA_MODE_PREMULTIPLIED),
        /* dpiX          */ dpi,
        /* dpiY          */ dpi
        );
        ComPtr<ID2D1Bitmap1> bitmap = {};
        THROW_IF_FAILED(Application::g_app->dx12Renderer()->d2d1DeviceContext()->CreateBitmapFromWicBitmap
        (
        /* wicBitmapSource  */ source.Get(),
        /* bitmapProperties */ props,
        /* bitmap           */ &bitmap
        ));
        return bitmap;
    }
}
