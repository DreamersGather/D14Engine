#include "Common/Precompile.h"

#include "TextPanelPage.h"

#include "Common/DirectXError.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "PageTemplate.h"

#include "IconLabelPage.h"
#include "LabelAreaPage.h"
#include "LabelPage.h"
#include "NumberBoxPage.h"
#include "TextBoxPage.h"
#include "TextEditorPage.h"

SharedPtr<TreeViewItem> createTextPanelPage
(std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>& pages)
{
    auto& ui_layout = (pages[L"Text Panel"] = makeUIObject<ConstraintLayout>());
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
        L"Text panel is used to display a series of texts. "
        L"You can simply use label(area) to display static (read-only) texts, "
        L"or more advancedly, use text-box/editor to make dynamic (editable) texts."
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
        IconLabel::compactLayout(L"Text Panel", nullptr, 1.0f, 11.0f, 23.0f));

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

    auto icon1 = loadBitmap(L"SideCategory/Light/TextPanel.png");
    auto icon2 = loadBitmap(L"SideCategory/Dark/TextPanel.png");

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

        PUSH_CHILD_ITEM_PAGE(Label, L"Label"),
        PUSH_CHILD_ITEM_PAGE(IconLabel, L"Icon Label"),
        PUSH_CHILD_ITEM_PAGE(LabelArea, L"Label Area"),
        PUSH_CHILD_ITEM_PAGE(TextBox, L"Text Box"),
        PUSH_CHILD_ITEM_PAGE(TextEditor, L"Text Editor"),
        PUSH_CHILD_ITEM_PAGE(NumberBox, L"Number Box")

#undef PUSH_CHILD_ITEM_PAGE
    };
    item->appendItem(childrenItems);
    item->setFolded(TreeViewItem::FOLDED);
    return item;
}
