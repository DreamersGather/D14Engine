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

ComPtr<ID2D1Bitmap1> loadBitmap(WstrParam bitmapPath)
{
    return bitmap_utils::loadBitmap(L"Test/UIKit/WidgetsGallery/Images/" + bitmapPath);
}
