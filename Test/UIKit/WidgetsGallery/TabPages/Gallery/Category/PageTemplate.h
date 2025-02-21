#pragma once

#include "Common/Precompile.h"

#include "UIKit/ConstraintLayout.h"
#include "UIKit/TreeViewItem.h"

d14engine::SharedPtr<d14engine::uikit::TreeViewItem> createPageTemplate
(std::unordered_map<d14engine::Wstring, d14engine::SharedPtr<d14engine::uikit::ConstraintLayout>>& pages);

D2D1_SIZE_F convert(const D2D1_SIZE_U& size);
D2D1_SIZE_U convert(const D2D1_SIZE_F& size);

d14engine::ComPtr<ID2D1Bitmap1> loadBitmap(d14engine::WstrParam bitmapPath);

// Type: MASTER, SLAVER
// Name: Category Name
// IconSize: 30X18, 26X26, 18X12
#define DEF_CATEGORY_ITEM(Type, Id, Name, IconSize) \
_MAKE_CATEGORY_ITEM(Name, IconSize) \
_CONFIGURE_CATEGORY_ITEM_ARROW_##Type \
_CONFIGURE_CATEGORY_ITEM_CONTENT(Id)

#define RET_CATEGORY_ITEM return item;

#define _ICON_PADDING_FOR_SIZE_30X9  9.0f, 21.0f
#define _ICON_PADDING_FOR_SIZE_30X18 9.0f, 21.0f
#define _ICON_PADDING_FOR_SIZE_26X26 11.0f, 23.0f
#define _ICON_PADDING_FOR_SIZE_20X20 14.0f, 26.0f
#define _ICON_PADDING_FOR_SIZE_18X12 15.0f, 27.0f

#define _MAKE_CATEGORY_ITEM(Name, IconSize) \
auto item = makeUIObject<TreeViewItem>( \
    IconLabel::compactLayout(L#Name, {}, _ICON_PADDING_FOR_SIZE_##IconSize));

#define _CONFIGURE_CATEGORY_ITEM_ARROW_MASTER \
item->getAppearance().arrow.geometry[(size_t)TreeViewItemState::Flag::Folded] = \
{ \
    { { 16.0f, 12.0f }, { 24.0f, 20.0f } }, /* line 0 */ \
    { { 24.0f, 20.0f }, { 16.0f, 28.0f } }  /* line 1 */ \
}; \
item->getAppearance().arrow.geometry[(size_t)TreeViewItemState::Flag::Unfolded] = \
{ \
    { { 13.0f, 16.0f }, { 20.0f, 24.0f } }, /* line 0 */ \
    { { 20.0f, 24.0f }, { 27.0f, 16.0f } }  /* line 1 */ \
};
#define _CONFIGURE_CATEGORY_ITEM_ARROW_SLAVER

#define _CONFIGURE_CATEGORY_ITEM_CONTENT(Id) \
item->resize(item->width(), 40.0f); \
auto& label = item->getContent<IconLabel>().lock()->label(); \
label->setTextFormat(D14_FONT(L"Default/Normal/14")); \
auto icon1 = loadBitmap(L"SideCategory/Light/" L#Id L".png"); \
auto icon2 = loadBitmap(L"SideCategory/Dark/" L#Id L".png"); \
item->f_onChangeThemeStyle = [=](Panel* p, const Panel::ThemeStyle& style) \
{ \
    auto content = ((TreeViewItem*)p)->getContent<IconLabel>().lock(); \
    if (style.mode == L"Light") content->icon.bitmap = icon1; \
    else if (style.mode == L"Dark") content->icon.bitmap = icon2; \
    content->icon.bitmap.interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR; \
    /* This ensures visual size of icons is consistent across different DPIs */ \
    content->icon.customSize = convert(content->icon.bitmap.data->GetPixelSize()); \
    content->updateLayout(); \
};

#define START_ADD_CHILDREN_ITEMS \
TreeViewItem::ChildItemList childrenItems = \
{
#define ADD_CHILD_ITEM_PAGE(Id, Name) \
create##Id##Page((pages[L#Name] = makeUIObject<ConstraintLayout>()).get())

#define END_ADD_CHILDREN_ITEMS \
}; \
item->appendItem(childrenItems); \
item->setFolded(TreeViewItem::FOLDED);
