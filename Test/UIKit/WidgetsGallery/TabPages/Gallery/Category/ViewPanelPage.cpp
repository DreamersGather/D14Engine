#include "Common/Precompile.h"

#include "ViewPanelPage.h"

#include "Common/DirectXError.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "PageTemplate.h"

#include "ListViewPage.h"
#include "ScrollViewPage.h"
#include "TreeViewPage.h"

SharedPtr<TreeViewItem> createViewPanelPage
(std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>& pages)
{
    auto& ui_layout = (pages[L"View Panel"] = makeUIObject<ConstraintLayout>());
    {
        ui_layout->maximalWidthHint = 1200.0f;
        ui_layout->resize(920.0f, 100.0f);

        auto& appear = ui_layout->getAppearance();
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
    auto ui_synopsisLabel = makeUIObject<Label>
    (
        L"View panel is used to display objects with large area or large number. "
        L"Specifically, scroll view is used to display objects with large area, "
        L"such as huge images; waterfall view is used to display a large number "
        L"of objects, which is usually arranged as a list or a tree."
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
    DEF_CATEGORY_ITEM(MASTER, ViewPanel, View Panel, 26X26)
    START_ADD_CHILDREN_ITEMS
    ADD_CHILD_ITEM_PAGE(ScrollView, Scroll View),
    ADD_CHILD_ITEM_PAGE(ListView, List View),
    ADD_CHILD_ITEM_PAGE(TreeView, Tree View)
    END_ADD_CHILDREN_ITEMS
    RET_CATEGORY_ITEM
}
