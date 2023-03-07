#include "Common/Precompile.h"

#include "ToggleButtonPage.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "PageTemplate.h"

SharedPtr<TreeViewItem> createToggleButtonPage(ConstraintLayout* page)
{
    // Initialize and populate the widget page with detailed content.
    {
        page->resize(920.0f, 1000.0f);

        auto& appear = page->getAppearance();
        appear.background.opacity = 1.0f;

        auto& light = appear.g_themeStyles.at(L"Light");
        {
            light.background.color = D2D1::ColorF{ 0xf9f9f9 };
        }
        auto& dark = appear.g_themeStyles.at(L"Dark");
        {
            dark.background.color = D2D1::ColorF{ 0x272727 };
        }
    }
    auto item = makeUIObject<TreeViewItem>(
        IconLabel::compactLayout(L"Toggle Button", nullptr, 1.0f, 9.0f, 21.0f));

    item->resize(item->width(), 40.0f);
    item->getContent<IconLabel>().lock()->label()->
        setTextFormat(D14_FONT(L"Default/Normal/14"));

    auto icon1 = loadBitmap(L"SideCategory/Light/ToggleButton.png");
    auto icon2 = loadBitmap(L"SideCategory/Dark/ToggleButton.png");

    item->f_onChangeTheme = [=](Panel* p, WstrParam themeName)
    {
        auto content = ((TreeViewItem*)p)->getContent<IconLabel>().lock();
        if (themeName == L"Light") content->icon.bitmap = icon1;
        else if (themeName == L"Dark") content->icon.bitmap = icon2;
        content->updateLayout();
    };
    return item;
}
