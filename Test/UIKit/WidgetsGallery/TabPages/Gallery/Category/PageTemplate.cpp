#include "Common/Precompile.h"

#include "PageTemplate.h"

#include "UIKit/BitmapUtils.h"

using namespace d14engine;
using namespace d14engine::uikit;

SharedPtr<TreeViewItem> createPageTemplate
(std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>& pages)
{
    return nullptr;
}

D2D1_SIZE_F convert(const D2D1_SIZE_U& size)
{
    return { (float)size.width, (float)size.height };
}

D2D1_SIZE_U convert(const D2D1_SIZE_F& size)
{
    return { (UINT32)size.width, (UINT32)size.height };
}

ComPtr<ID2D1Bitmap1> loadBitmap(WstrParam bitmapPath)
{
    return bitmap_utils::loadBitmap(L"Test/UIKit/WidgetsGallery/Images/" + bitmapPath);
}
