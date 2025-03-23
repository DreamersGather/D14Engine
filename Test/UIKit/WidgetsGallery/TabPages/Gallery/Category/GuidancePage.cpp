#include "Common/Precompile.h"

#include "GuidancePage.h"

#include "Common/DirectXError.h"

#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "PageTemplate.h"

SharedPtr<TreeViewItem> createGuidancePage
(std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>& pages)
{
    auto& ui_layout = (pages[L"Guidance"] = makeUIObject<ConstraintLayout>());
    {
        ui_layout->maximalWidthHint = 1200.0f;
        ui_layout->setSize(920.0f, 400.0f);

        auto& appear = ui_layout->appearance();
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
        L"UIKit @ D14Engine is initially developed for creating the Editor part "
        L"of D14Engine, and with the development, it has been becoming a common "
        L"UI library on Windows-desktop.\n\n"
        L"The kit is designed to match Fluent Design, so it looks well with "
        L"the native Windows-desktop environment. It is worth noting that you can "
        L"customize all UI widgets by modifying their Appearance parts, or even "
        L"create a brand-new widget by inheriting the most derived widget class "
        L"(i.e. Panel) and overriding the specific virtual methods.\n\n"
        L"This demo-app (Widgets Gallery) is developed to show the basic usage "
        L"of the commonly used widgets in the library. Feel free to look around."
    );
    {
        ui_synopsisLabel->setTextFormat(D14_FONT(L"Default/14"));
        ui_synopsisLabel->hardAlignment.vert = Label::VertAlignment::Top;
        THROW_IF_FAILED(ui_synopsisLabel->textLayout()->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP));

        THROW_IF_FAILED(ui_synopsisLabel->textLayout()->SetUnderline(true, { 0, 17 }));
        THROW_IF_FAILED(ui_synopsisLabel->textLayout()->SetFontSize(18.0f * 96.0f / 72.0f, { 0, 17 }));

        ConstraintLayout::GeometryInfo geoInfo = {};

        ui_synopsisLabel->appearance().background = { D2D1::ColorF{ 0xff0000 }, 1.0f };

        geoInfo.keepWidth = false;
        geoInfo.Left.ToLeft = 70.0f;
        geoInfo.Right.ToRight = 70.0f;

        geoInfo.keepHeight = false;
        geoInfo.Top.ToTop = 20.0f;
        geoInfo.Bottom.ToTop = 20.0f;

        ui_layout->addElement(ui_synopsisLabel, geoInfo);
    }
    auto item = makeUIObject<TreeViewItem>(L"Guidance", math_utils::heightOnlyRect(40.0f));
    item->getContent<IconLabel>().lock()->label()->setTextFormat(D14_FONT(L"Default/14"));
    return item;
}
