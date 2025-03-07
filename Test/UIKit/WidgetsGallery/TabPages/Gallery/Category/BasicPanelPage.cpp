#include "Common/Precompile.h"

#include "BasicPanelPage.h"

#include "Common/DirectXError.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "PageTemplate.h"

#include "ClickablePanelPage.h"
#include "DraggablePanelPage.h"
#include "ResizablePanelPage.h"

SharedPtr<TreeViewItem> createBasicPanelPage
(std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>& pages)
{
    auto& ui_layout = (pages[L"Basic Panel"] = makeUIObject<ConstraintLayout>());
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
        L"Panel is the most derived UI object type in UIKit @ D14Engine library, "
        L"and basic panels are those that be of some ubiquitous attributes, such "
        L"as clickable, draggable and resizable etc... These basic types make it "
        L"convenient to implement a new widget."
    );
    {
        ui_synopsisLabel->setTextFormat(D14_FONT(L"Default/14"));
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
    DEF_CATEGORY_ITEM(MASTER, BasicPanel, Basic Panel, 26X26)
    START_ADD_CHILDREN_ITEMS
    ADD_CHILD_ITEM_PAGE(ClickablePanel, Clickable Panel),
    ADD_CHILD_ITEM_PAGE(DraggablePanel, Draggable Panel),
    ADD_CHILD_ITEM_PAGE(ResizablePanel, Resizable Panel)
    END_ADD_CHILDREN_ITEMS
    RET_CATEGORY_ITEM
}
