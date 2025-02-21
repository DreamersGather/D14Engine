#include "Common/Precompile.h"

#include "FilledButtonPage.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

#include "PageTemplate.h"

using namespace d14engine;
using namespace d14engine::uikit;

SharedPtr<TreeViewItem> createFilledButtonPage(ConstraintLayout* page)
{
    // Initialize and populate the widget page with detailed content.
    {
        page->resize(920.0f, 1000.0f);

        auto& appear = page->getAppearance();
        appear.background.opacity = 1.0f;

        auto& light = appear.g_themeData.at(L"Light");
        {
            light.background.color = D2D1::ColorF{ 0xf9f9f9 };
        }
        auto& dark = appear.g_themeData.at(L"Dark");
        {
            dark.background.color = D2D1::ColorF{ 0x272727 };
        }
    }
    DEF_CATEGORY_ITEM(SLAVER, FilledButton, Filled Button, 30X18)
    RET_CATEGORY_ITEM
}
