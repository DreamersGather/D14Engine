#pragma once

#include "Common/Precompile.h"

namespace d14engine::uikit::bitmap_utils
{
    void saveBitmap(ID2D1Bitmap1* image, WstrRefer imagePath, const GUID& format = GUID_ContainerFormatPng);

    ComPtr<ID2D1Bitmap1> loadBitmap(UINT width, UINT height, BYTE* data = nullptr, D2D1_BITMAP_OPTIONS options = D2D1_BITMAP_OPTIONS_NONE);

    ComPtr<ID2D1Bitmap1> loadBitmap(WstrRefer imagePath, D2D1_BITMAP_OPTIONS options = D2D1_BITMAP_OPTIONS_NONE);

    ComPtr<ID2D1Bitmap1> loadPackedBitmap(WstrRefer resName, WstrRefer resType = L"PNG", D2D1_BITMAP_OPTIONS options = D2D1_BITMAP_OPTIONS_NONE);
}
