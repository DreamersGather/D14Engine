#include "Common/Precompile.h"

#include "Gallery.h"

#include "Common/DirectXError.h"

#include "UIKit/BitmapUtils.h"
#include "UIKit/FlatButton.h"
#include "UIKit/IconLabel.h"
#include "UIKit/TextBox.h"
#include "UIKit/TreeView.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "Category/BasicPanelPage.h"
#include "Category/ButtonPage.h"
#include "Category/GuidancePage.h"
#include "Category/LayoutPage.h"
#include "Category/MiscellaneousPage.h"
#include "Category/PageTemplate.h"
#include "Category/TextPanelPage.h"
#include "Category/ViewPanelPage.h"

void createGalleryTabPage(ConstraintLayout* page)
{
    auto ui_sideLayout = makeUIObject<ConstraintLayout>();
    {
        ConstraintLayout::GeometryInfo geoInfo = {};

        geoInfo.keepWidth = false;
        geoInfo.Left.ToLeft = 0.0f;
        geoInfo.Right.ToLeft = 350.0f;

        geoInfo.keepHeight = false;
        geoInfo.Top.ToTop = 24.0f;
        geoInfo.Bottom.ToBottom = 0.0f;

        page->addElement(ui_sideLayout, geoInfo);
    }
    auto ui_searcher = makeUIObject<Panel>();
    auto ui_searchBox = makeManagedUIObject<TextBox>(ui_searcher, 5.0f);
    {
        ui_searcher->resize(302.0f, 40.0f);

        ui_searcher->forceSingleMouseEnterLeaveEvent = false;

        ConstraintLayout::GeometryInfo geoInfo = {};
        geoInfo.Left.ToLeft = 24.0f;
        geoInfo.Top.ToTop = 0.0f;
        ui_sideLayout->addElement(ui_searcher, geoInfo);

        ui_searchBox->transform(ui_searcher->selfCoordRect());

        ui_searchBox->setTextFormat(D14_FONT(L"Default/Normal/14"));
        ui_searchBox->setVisibleTextRect({ 5.0f, 8.0f, 212.0f, 32.0f });

        auto& ui_hintLabel = ui_searchBox->placeholder();
        ui_hintLabel->setText(L"Search...");
        ui_hintLabel->setTextFormat(D14_FONT(L"Default/Normal/14"));
        THROW_IF_FAILED(ui_hintLabel->textLayout()->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, { 0, UINT32_MAX }));

        auto keepSearchBoxFocus =
        [
            wk_searchBox = (WeakPtr<TextBox>)ui_searchBox
        ]
        (Panel* p, MouseMoveEvent& e)
        {
            if (!wk_searchBox.expired())
            {
                wk_searchBox.lock()->appEventReactability.focus.lose = false;
            }
        };
        auto restoreSearchBoxFocus =
        [
            wk_searchBox = (WeakPtr<TextBox>)ui_searchBox
        ]
        (Panel* p, MouseMoveEvent& e)
        {
            if (!wk_searchBox.expired())
            {
                wk_searchBox.lock()->appEventReactability.focus.lose = true;
            }
        };
        auto ui_clearButton = makeManagedUIObject<FlatButton>(
            ui_searcher, IconLabel::iconExpandedLayout(), 5.0f);

        ui_clearButton->transform(224.0f, 8.0f, 30.0f, 24.0f);
        ui_clearButton->moveAbovePeerObject(ui_searchBox.get());

        auto clearIconLight = loadBitmap(L"SearchBox/Light/Clear.png");
        auto clearIconDark = loadBitmap(L"SearchBox/Dark/Clear.png");

        ui_clearButton->f_onChangeTheme = [=](Panel* p, WstrParam themeName)
        {
            auto& content = dynamic_cast<Button*>(p)->content();
            if (themeName == L"Light") content->icon.bitmap = clearIconLight;
            else if (themeName == L"Dark") content->icon.bitmap = clearIconDark;
            content->icon.bitmap.interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
            // This ensures visual size of icons is consistent across different DPIs
            content->icon.customSize = convert(content->icon.bitmap.data->GetPixelSize());
            content->updateLayout();
        };
        ui_clearButton->f_onMouseEnter = keepSearchBoxFocus;
        ui_clearButton->f_onMouseLeave = restoreSearchBoxFocus;
        ui_clearButton->f_onMouseButtonRelease =
        [
            wk_searchBox = (WeakPtr<TextBox>)ui_searchBox
        ]
        (ClickablePanel* clkp, ClickablePanel::Event& e)
        {
            if (!wk_searchBox.expired())
            {
                auto sh_searchBox = wk_searchBox.lock();
                sh_searchBox->setText(L"");
                sh_searchBox->setIndicatorPosition(0);
                sh_searchBox->setHiliteRange({ 0, 0 });
            }
        };
        auto ui_searchButton = makeManagedUIObject<FlatButton>(
            ui_searcher, IconLabel::iconExpandedLayout(), 5.0f);

        ui_searchButton->transform(260.0f, 8.0f, 30.0f, 24.0f);
        ui_searchButton->moveAbovePeerObject(ui_searchBox.get());

        auto searchIconLight = loadBitmap(L"SearchBox/Light/Search.png");
        auto searchIconDark = loadBitmap(L"SearchBox/Dark/Search.png");

        ui_searchButton->f_onChangeTheme = [=](Panel* p, WstrParam themeName)
        {
            auto& content = dynamic_cast<Button*>(p)->content();
            if (themeName == L"Light") content->icon.bitmap = searchIconLight;
            else if (themeName == L"Dark") content->icon.bitmap = searchIconDark;
            content->icon.bitmap.interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
            // This ensures visual size of icons is consistent across different DPIs
            content->icon.customSize = convert(content->icon.bitmap.data->GetPixelSize());
            content->updateLayout();
        };
        ui_searchButton->f_onMouseEnter = keepSearchBoxFocus;
        ui_searchButton->f_onMouseLeave = restoreSearchBoxFocus;
        ui_searchButton->f_onMouseButtonRelease =
        [
            wk_searchBox = (WeakPtr<TextBox>)ui_searchBox
        ]
        (ClickablePanel* clkp, ClickablePanel::Event& e)
        {
            if (!wk_searchBox.expired())
            {
                auto sh_searchBox = wk_searchBox.lock();
                KeyboardEvent kbe = {};
                kbe.vkey = VK_RETURN;
                kbe.state.flag = KeyboardEvent::State::Flag::Released;
                sh_searchBox->f_onKeyboard(sh_searchBox.get(), kbe);
            }
        };
    }
    auto ui_sideCategory = makeUIObject<TreeView>();
    using CategoryPageMap = std::unordered_map<Wstring, SharedPtr<ConstraintLayout>>;
    auto categoryPages = std::make_shared<CategoryPageMap>();
    {
        ui_sideCategory->deltaPixelsPerScroll.vert = 40.0f;

        ui_sideCategory->selectMode = TreeView::SelectMode::Single;

        ui_sideCategory->setBaseHorzIndent(40.0f);
        ui_sideCategory->setHorzIndentEachNodelLevel(40.0f);

        ConstraintLayout::GeometryInfo geoInfo = {};

        geoInfo.keepWidth = false;
        geoInfo.Left.ToLeft = 0.0f;
        geoInfo.Right.ToRight = -1.0f;   

        geoInfo.keepHeight = false;
        geoInfo.Top.ToTop = 64.0f;
        geoInfo.Bottom.ToBottom = 0.0f;

        ui_sideLayout->addElement(ui_sideCategory, geoInfo);

        TreeView::ItemList items =
        {
            createGuidancePage(*categoryPages),
            createBasicPanelPage(*categoryPages),
            createButtonPage(*categoryPages),
            createLayoutPage(*categoryPages),
            createViewPanelPage(*categoryPages),
            createTextPanelPage(*categoryPages),
            createMiscellaneousPage(*categoryPages)
        };
        ui_sideCategory->appendRootItem(items);

        ui_sideCategory->f_onChangeTheme = [=](Panel* p, WstrParam themeName)
        {
            for (auto& page : *categoryPages)
            {
                page.second->onChangeTheme(themeName);
            }
        };
    }
    auto ui_contentLayout = makeUIObject<ConstraintLayout>();
    {
        auto& appear = ui_contentLayout->getAppearance();

        appear.background.opacity = 1.0f;
        appear.stroke.width = 2.0f;
        appear.stroke.opacity = 1.0f;

        auto& light = appear.g_themeStyles.at(L"Light");
        {
            light.background.color = D2D1::ColorF{ 0xf9f9f9 };
        }
        auto& dark = appear.g_themeStyles.at(L"Dark");
        {
            dark.background.color = D2D1::ColorF{ 0x272727 };
        }
        appear.changeTheme(L"Light");

        ui_contentLayout->roundRadiusX = ui_contentLayout->roundRadiusY = 10.0f;

        ConstraintLayout::GeometryInfo geoInfo = {};

        geoInfo.keepWidth = false;
        geoInfo.Left.ToLeft = 350.0f;
        geoInfo.Right.ToRight = -10.0f;

        geoInfo.keepHeight = false;
        geoInfo.Top.ToTop = 24.0f;
        geoInfo.Bottom.ToBottom = -10.0f;

        page->addElement(ui_contentLayout, geoInfo);
    }
    auto ui_topTitle = makeUIObject<Label>(L"Guidance");
    {
        ui_topTitle->setTextFormat(D14_FONT(L"Default/SemiBold/22"));
        THROW_IF_FAILED(ui_topTitle->textLayout()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

        ConstraintLayout::GeometryInfo geoInfo = {};
        geoInfo.Left.ToLeft = 70.0f;
        geoInfo.Top.ToTop = 50.0f;
        ui_contentLayout->addElement(ui_topTitle, geoInfo);
    }
    auto ui_bottomView = makeUIObject<ScrollView>(categoryPages->at(L"Guidance"));
    {
        ui_bottomView->deltaPixelsPerScroll.vert = 90.0f;

        ConstraintLayout::GeometryInfo geoInfo = {};

        geoInfo.keepWidth = false;
        geoInfo.Left.ToLeft = 0.0f;
        geoInfo.Right.ToRight = 20.0f;

        geoInfo.keepHeight = false;
        geoInfo.Top.ToTop = 100.0f;
        geoInfo.Bottom.ToBottom = 30.0f;

        ui_contentLayout->addElement(ui_bottomView, geoInfo);

        ui_bottomView->f_onSize = [](Panel* p, SizeEvent& e)
        {
            auto content = dynamic_cast<ScrollView*>(p)->content();
            if (!content.expired())
            {
                auto pContent = content.lock();
                pContent->resize(e.size.width, pContent->height());
            }
        };
    }
    ui_searchBox->f_onKeyboard =
    [
        categoryPages,
        wk_topTitle = (WeakPtr<Label>)ui_topTitle,
        wk_bottomView = (WeakPtr<ScrollView>)ui_bottomView
    ]
    (Panel* p, KeyboardEvent& e)
    {
        // The following code contains a bug:
        // When using an input method that requires secondary selection, the Enter key
        // used toconfirm a candidate word will also trigger the search action. (´･_･`)
        if (e.vkey == VK_RETURN && e.state.released())
        {
            auto& categoryName = ((TextBox*)p)->text();
            auto pageItor = categoryPages->find(categoryName);
            if (pageItor != categoryPages->end())
            {
                if (!wk_topTitle.expired())
                {
                    wk_topTitle.lock()->setText(categoryName);
                }
                if (!wk_bottomView.expired())
                {
                    wk_bottomView.lock()->setContent(pageItor->second);
                }
            }
            else if (!categoryName.empty())
            {
                if (!wk_topTitle.expired())
                {
                    wk_topTitle.lock()->setText(L"\"" + categoryName + L"\" not found...");
                }
                if (!wk_bottomView.expired())
                {
                    wk_bottomView.lock()->setContent(nullptr);
                }
            }
        }
    };
    ui_sideCategory->f_onSelectChange =
    [
        categoryPages,
        wk_topTitle = (WeakPtr<Label>)ui_topTitle,
        wk_bottomView = (WeakPtr<ScrollView>)ui_bottomView
    ]
    (TreeView::WaterfallView* view, const TreeView::ItemIndexSet& selected)
    {
        if (!selected.empty())
        {
            auto currItem = (**selected.begin())->getContent<IconLabel>();
            if (!currItem.expired())
            {
                auto& categoryName = currItem.lock()->label()->text();
                if (!wk_topTitle.expired())
                {
                    wk_topTitle.lock()->setText(categoryName);
                }
                auto pageItor = categoryPages->find(categoryName);
                if (pageItor != categoryPages->end())
                {
                    if (!wk_bottomView.expired())
                    {
                        auto sh_bottomView = wk_bottomView.lock();
                        pageItor->second->resize(sh_bottomView->width(), pageItor->second->height());
                        sh_bottomView->setContent(pageItor->second);
                    }
                }
            }
        }
    };
}
