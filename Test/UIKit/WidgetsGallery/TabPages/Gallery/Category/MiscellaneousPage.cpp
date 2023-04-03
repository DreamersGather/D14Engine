#include "Common/Precompile.h"

#include "MiscellaneousPage.h"

#include "Common/DirectXError.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "PageTemplate.h"

#include "CheckBoxPage.h"
#include "OnOffSwitchPage.h"
#include "SliderPage.h"
#include "TabGroupPage.h"
#include "WindowPage.h"

SharedPtr<TreeViewItem> createMiscellaneousPage
(std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>& pages)
{
    auto& ui_layout = (pages[L"Miscellaneous"] = makeUIObject<ConstraintLayout>());
    {
        ui_layout->maximalWidthHint = 1200.0f;
        ui_layout->resize(920.0f, 100.0f);

        auto& appear = ui_layout->getAppearance();
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
    auto ui_synopsisLabel = makeUIObject<Label>
    (
        L"Other useful widgets are classified into this cateogry."
    );
    {
        ui_synopsisLabel->setTextFormat(D14_FONT(L"Default/Normal/14"));
        ui_synopsisLabel->hardAlignment.vert = Label::VertAlignment::Top;
        THROW_IF_FAILED(ui_synopsisLabel->textLayout()->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP));
        
        ConstraintLayout::GeometryInfo geoInfo = {};

        ui_synopsisLabel->getAppearance().background = { D2D1::ColorF{ 0xff0000 }, 1.0f };

        geoInfo.keepWidth = false;
        geoInfo.Left.ToLeft = 70.0f;
        geoInfo.Right.ToRight = 70.0f;

        geoInfo.keepHeight = false;
        geoInfo.Top.ToTop = 20.0f;
        geoInfo.Bottom.ToTop = 20.0f;

        ui_layout->addElement(ui_synopsisLabel, geoInfo);
    }
    auto item = makeUIObject<TreeViewItem>(
        IconLabel::compactLayout(L"Miscellaneous", nullptr, 1.0f, 15.0f, 27.0f));

    item->getAppearance().arrow.geometry[(size_t)TreeViewItemState::Flag::Folded] =
    {
        { { 16.0f, 12.0f }, { 24.0f, 20.0f } }, // line 0
        { { 24.0f, 19.0f }, { 16.0f, 27.0f } }  // line 1
    };
    item->getAppearance().arrow.geometry[(size_t)TreeViewItemState::Flag::Unfolded] =
    {
        { { 13.0f, 16.0f }, { 21.0f, 24.0f } }, // line 0
        { { 20.0f, 24.0f }, { 28.0f, 16.0f } }  // line 1
    };
    item->resize(item->width(), 40.0f);
    item->getContent<IconLabel>().lock()->label()->
        setTextFormat(D14_FONT(L"Default/Normal/14"));

    auto icon1 = loadBitmap(L"SideCategory/Light/Miscellaneous.png");
    auto icon2 = loadBitmap(L"SideCategory/Dark/Miscellaneous.png");

    item->f_onChangeTheme = [=](Panel* p, WstrParam themeName)
    {
        auto content = ((TreeViewItem*)p)->getContent<IconLabel>().lock();
        if (themeName == L"Light") content->icon.bitmap = icon1;
        else if (themeName == L"Dark") content->icon.bitmap = icon2;
        content->updateLayout();
    };
    TreeViewItem::ChildItemList childrenItems =
    {
#define PUSH_CHILD_ITEM_PAGE(Id, Name) \
    create##Id##Page((pages[Name] = \
    makeUIObject<ConstraintLayout>()).get())

        PUSH_CHILD_ITEM_PAGE(CheckBox, L"Check Box"),
        PUSH_CHILD_ITEM_PAGE(OnOffSwitch, L"On-Off Switch"),
        PUSH_CHILD_ITEM_PAGE(Slider, L"Slider"),
        PUSH_CHILD_ITEM_PAGE(TabGroup, L"Tab Group"),
        PUSH_CHILD_ITEM_PAGE(Window, L"Window")

#undef PUSH_CHILD_ITEM_PAGE
    };
    item->appendItem(childrenItems);
    item->setFolded(TreeViewItem::FOLDED);
    return item;
}
