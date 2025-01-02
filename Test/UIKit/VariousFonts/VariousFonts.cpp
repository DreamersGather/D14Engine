#include "Common/Precompile.h"

#include "Common/DirectXError.h"
#include "Common/MathUtils/Basic.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/ComboBox.h"
#include "UIKit/FileSystemUtils.h"
#include "UIKit/GridLayout.h"
#include "UIKit/HorzSlider.h"
#include "UIKit/IconLabel.h"
#include "UIKit/LabelArea.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/PopupMenu.h"
#include "UIKit/TabCaption.h"
#include "UIKit/TabGroup.h"

using namespace d14engine;
using namespace d14engine::uikit;

D14_SET_APP_ENTRY(mainVariousFonts)
{
    Application::CreateInfo info = {};
    if (argc >= 2 && lstrcmp(argv[1], L"HighDPI") == 0)
    {
        info.dpi = 192.0f;
    }
    else info.dpi = 96.0f;
    info.win32WindowRect = { 0, 0, 800, 600 };

    BitmapObject::g_interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;

    return Application(argc, argv, info).run([&](Application* app)
    {
        auto textDrawMode = app->dxRenderer()->getDefaultTextRenderingMode();
        textDrawMode.renderingMode = DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC;
        app->dxRenderer()->setTextRenderingMode(textDrawMode);

        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - VariousFonts @ UIKit");
        {
            ui_mainWindow->moveTopmost();
            ui_mainWindow->isMaximizeEnabled = false;

            ui_mainWindow->caption()->transform(300.0f, 0.0f, 376.0f, 32.0f);
        }
        auto ui_darkModeLabel = makeRootUIObject<Label>(L"Dark Mode");
        auto ui_darkModeSwitch = makeRootUIObject<OnOffSwitch>();
        {
            ui_darkModeLabel->moveTopmost();
            ui_darkModeLabel->transform(10.0f, 0.0f, 120.0f, 32.0f);

            ui_darkModeSwitch->moveTopmost();
            ui_darkModeSwitch->move(130.0f, 4.0f);

            if (app->systemThemeStyle().mode == Application::ThemeStyle::Mode::Light)
            {
                ui_darkModeSwitch->setOnOffState(OnOffSwitch::OFF);
            }
            else ui_darkModeSwitch->setOnOffState(OnOffSwitch::ON);

            app->customThemeStyle = app->systemThemeStyle();
            app->f_onSystemThemeStyleChange = [app]
            {
                app->customThemeStyle.value().color = app->systemThemeStyle().color;
                app->changeTheme(app->currThemeName());
            };
            ui_darkModeSwitch->f_onStateChange = [app]
            (OnOffSwitch::StatefulObject* obj, OnOffSwitch::StatefulObject::Event& e)
            {
                auto& customThemeStyle = app->customThemeStyle.value();
                if (e.on())
                {
                    customThemeStyle.mode = Application::ThemeStyle::Mode::Dark;
                    app->changeTheme(L"Dark");
                }
                else if (e.off())
                {
                    customThemeStyle.mode = Application::ThemeStyle::Mode::Light;
                    app->changeTheme(L"Light");
                }
            };
        }
        auto ui_screenshot = makeRootUIObject<OutlinedButton>(L"Screenshot");
        {
            ui_screenshot->moveTopmost();
            ui_screenshot->transform(200.0f, 4.0f, 100.0f, 24.0f);
            ui_screenshot->content()->label()->setTextFormat(D14_FONT(L"Default/Normal/12"));

            ui_screenshot->f_onMouseButtonRelease = [app]
            (ClickablePanel* clkp, ClickablePanel::Event& e)
            {
                auto image = app->screenshot();
                CreateDirectory(L"Screenshots", nullptr);
                bitmap_utils::saveBitmap(image.Get(), L"Screenshots/VariousFonts.png");
            };
        }
        auto ui_clientArea = makeUIObject<Panel>();
        {
            ui_mainWindow->setCenterUIObject(ui_clientArea);
        }
        auto ui_textViewer = makeManagedUIObject<TabGroup>(ui_clientArea);
        auto wk_textViewer = (WeakPtr<TabGroup>)ui_textViewer;
        {
            ui_textViewer->transform(0.0f, 50.0f, 800.0f, 300.0f);

#define SET_CARD_SIZE(State, Width, Height) \
    ui_textViewer->getAppearance().tabBar.card.main \
    [(size_t)TabGroup::CardState::State].geometry.size = { Width, Height }

            SET_CARD_SIZE(Dormant, 250.0f, 32.0f);
            SET_CARD_SIZE(Hover, 250.0f, 32.0f);
            SET_CARD_SIZE(Active, 266.0f, 40.0f);

#undef SET_CARD_SIZE
            ui_textViewer->loadActiveCardShadowBitmap();

            auto& barAppear = ui_textViewer->getAppearance().tabBar;

            barAppear.geometry.height = 40.0f;
            barAppear.separator.geometry.size.height = 24.0f;
            barAppear.moreCards.control.button.geometry.offset.y = 7.0f;
        }
#ifdef _DEBUG
        Wstring excerptDir = L"Test/UIKit/VariousFonts/";
#else
        Wstring excerptDir = L""; // same path with the executable
#endif
        auto getFileSize = [](WstrParam filePath) -> size_t
        {
            struct _stat fileinfo;
            _wstat(filePath.c_str(), &fileinfo);
            return fileinfo.st_size;
        };
        file_system_utils::foreachFileInDir(excerptDir, L"*.txt", [&](WstrParam filePath)
        {
            auto fileName = file_system_utils::extractFileName(filePath);
            auto filePrefix = file_system_utils::extractFilePrefix(fileName);

            bool isLangZhCn = file_system_utils::extractFileSuffix(filePrefix) == L"zh_CN";

            auto& formats = resource_utils::g_textFormats;
            auto captionFormat = formats.at(isLangZhCn ? L"默认/正常/14" : L"Default/Normal/14").Get();
            auto contentFormat = formats.at(isLangZhCn ? L"默认/正常/16" : L"Default/Normal/16").Get();

            auto contentWrapping = isLangZhCn ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP;
            auto contentAlignment = isLangZhCn ? DWRITE_TEXT_ALIGNMENT_LEADING : DWRITE_TEXT_ALIGNMENT_CENTER;

            auto ui_caption = makeUIObject<TabCaption>(filePrefix);

            ui_caption->getAppearance().title.rightPadding = 12.0f;

            ui_caption->closable = false;
            ui_caption->title()->label()->setTextFormat(captionFormat);

#pragma warning(push)
#pragma warning(disable : 4996)
            FILE* fileHandle = _wfopen(filePath.c_str(), L"r, ccs=UNICODE");
#pragma warning(pop)
            THROW_IF_NULL(fileHandle);

            Wstring fileBuffer = {};
            auto fileSize = getFileSize(filePath);
            if (fileSize > 0)
            {
                fileBuffer.resize(fileSize);
                fread(fileBuffer.data(), sizeof(WCHAR), fileSize, fileHandle);
            }
            auto ui_block = makeUIObject<LabelArea>(fileBuffer, math_utils::widthOnlyRect(750.0f));
            auto wk_block = (WeakPtr<LabelArea>)ui_block;

            ui_block->setTextFormat(contentFormat);
            THROW_IF_FAILED(ui_block->textLayout()->SetWordWrapping(contentWrapping));
            THROW_IF_FAILED(ui_block->textLayout()->SetTextAlignment(contentAlignment));

            auto ui_wrapper = makeUIObject<ConstraintLayout>();
            ui_wrapper->resize(800.0f, ui_block->textMetrics().height + 100.0f);

            ConstraintLayout::GeometryInfo geoInfo = {};

            geoInfo.keepWidth = false;
            geoInfo.Left.ToLeft = 25.0f;
            geoInfo.Right.ToRight = 25.0f;

            geoInfo.keepHeight = false;
            geoInfo.Top.ToTop = 50.0f;
            geoInfo.Bottom.ToBottom = 50.0f;

            ui_wrapper->addElement(ui_block, geoInfo);

            auto ui_content = makeUIObject<ScrollView>(ui_wrapper);

            // Set opaque background to support rendering ClearType text.
            ui_content->getAppearance().background.opacity = 1.0f;

            // Keep the hilite range when scrolling view with mouse button.
            ui_content->f_onStartThumbScrolling = [=]
            (ScrollView* sv, const D2D1_POINT_2F& offset)
            {
                if (!wk_block.expired())
                {
                    wk_block.lock()->keepHiliteRange = true;
                }
            };
            ui_content->f_onEndThumbScrolling = [=]
            (ScrollView* sv, const D2D1_POINT_2F& offset)
            {
                if (!wk_block.expired())
                {
                    auto sh_block = wk_block.lock();
                    app->focusUIObject(sh_block);
                    sh_block->keepHiliteRange = false;
                }
            };
            ui_textViewer->appendTab({ ui_caption, ui_content });
            return false;
        });
        ui_textViewer->selectTab(0);

        // Load dependent text formats.

        std::vector<Wstring> fontNameArray =
        {
            L"Segoe UI", L"Arial", L"Times New Roman",
            L"微软雅黑", L"楷体", L"宋体"
        };
        std::vector<Wstring> fontLocaleNameArray =
        {
            L"en-us", L"en-us", L"en-us", L"zh-cn", L"zh-cn", L"zh-cn"
        };
        std::vector<Wstring> fontWeightArray =
        {
            L"Light", L"Semi-Light", L"Normal", L"Semi-Bold", L"Bold"
        };
        std::vector<DWRITE_FONT_WEIGHT> fontWeightEnumArray =
        {
            DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_WEIGHT_SEMI_LIGHT, DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_WEIGHT_BOLD
        };
        auto textFormatMap = std::make_shared<resource_utils::TextFormatMap>();
        for (size_t n = 0; n < fontNameArray.size(); ++n)
        {
            for (size_t w = 0; w < fontWeightArray.size(); ++w)
            {
                for (int fontSize = 11; fontSize <= 32; ++fontSize)
                {
                    auto textFormatName =
                        fontNameArray[n] + L"/" +
                        fontWeightArray[w] + L"/" +
                        std::to_wstring(fontSize);

                    auto factory = app->dxRenderer()->dwriteFactory();
                    THROW_IF_FAILED(factory->CreateTextFormat(
                        fontNameArray[n].c_str(),
                        nullptr,
                        fontWeightEnumArray[w],
                        DWRITE_FONT_STYLE_NORMAL,
                        DWRITE_FONT_STRETCH_NORMAL,
                        // 1 inch == 72 pt == 96 dip
                        fontSize * 96.0f / 72.0f,
                        fontLocaleNameArray[n].c_str(),
                        &(*textFormatMap)[textFormatName]));
                }
            }
        }
        // Create bottom control-panel.

        auto ui_controlPanel = makeManagedUIObject<GridLayout>(ui_clientArea);
        {
            ui_controlPanel->transform(0.0f, 374.0f, 800.0f, 190.0f);
            ui_controlPanel->getAppearance().background.opacity = 1.0f;
            ui_controlPanel->setCellCount(10, 2);
        }
        auto ui_fontNameSelector = makeUIObject<ComboBox>(5.0f);
        auto wk_fontNameSelector = (WeakPtr<ComboBox>)ui_fontNameSelector;
        {
            ui_fontNameSelector->resize(240.0f, 40.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 1, 4 };
            geoInfo1.axis.y = { 0, 1 };
            ui_controlPanel->addElement(ui_fontNameSelector, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Text font");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 0, 1 };
            geoInfo2.axis.y = { 0, 1 };
            geoInfo2.spacing.left = 24.0f;
            ui_controlPanel->addElement(ui_label, geoInfo2);

            PopupMenu::ItemList fontNameItems = {};
            for (auto& fontName : fontNameArray)
            {
                auto content = IconLabel::comboBoxLayout(fontName);

                content->label()->setTextFormat(
                    textFormatMap->at(fontName + L"/Normal/16").Get());

                fontNameItems.push_back(makeUIObject<MenuItem>(
                    content, math_utils::heightOnlyRect(40.0f)));
            }
            auto& dropDownMenu = ui_fontNameSelector->dropDownMenu();

            dropDownMenu->resize(dropDownMenu->width(), 240.0f);
            dropDownMenu->appendItem(fontNameItems);

            ui_fontNameSelector->customMenuRelativePosition = { 0.0f, -240.0f };
            ui_fontNameSelector->setCurrSelected(0);
        }
        auto ui_textAntialiasModeSelector = makeUIObject<ComboBox>(5.0f);
        {
            ui_textAntialiasModeSelector->resize(160.0f, 40.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 7, 3 };
            geoInfo1.axis.y = { 0, 1 };
            ui_controlPanel->addElement(ui_textAntialiasModeSelector, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Text antialias mode");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 5, 2 };
            geoInfo2.axis.y = { 0, 1 };
            ui_controlPanel->addElement(ui_label, geoInfo2);

            std::vector<std::pair<Wstring, D2D1_TEXT_ANTIALIAS_MODE>> strModeArray =
            {
                { L"Default", D2D1_TEXT_ANTIALIAS_MODE_DEFAULT },
                { L"ClearType", D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE },
                { L"Grayscale", D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE }
            };
            PopupMenu::ItemList strModeItems = {};
            for (auto& strMode : strModeArray)
            {
                strModeItems.push_back(makeUIObject<MenuItem>(
                    IconLabel::comboBoxLayout(strMode.first), math_utils::heightOnlyRect(40.0f)));
            }
            auto& dropDownMenu = ui_textAntialiasModeSelector->dropDownMenu();

            dropDownMenu->resize(dropDownMenu->width(), 120.0f);
            dropDownMenu->appendItem(strModeItems);

            ui_textAntialiasModeSelector->setCurrSelected(0);

            using StrModeMap = std::unordered_map<Wstring, D2D1_TEXT_ANTIALIAS_MODE>;
            ui_textAntialiasModeSelector->f_onSelectedChange =
            [=,
                strModeMap = StrModeMap{ strModeArray.begin(), strModeArray.end() }
            ]
            (ComboBox* cb, IconLabel* content)
            {
                app->dxRenderer()->setTextAntialiasMode(strModeMap.at(content->label()->text()));
            };
        }
        auto ui_fontSizeSlider = makeUIObject<HorzSlider>();
        auto wk_fontSizeSlider = (WeakPtr<HorzSlider>)ui_fontSizeSlider;
        {
            ui_fontSizeSlider->resize(210.0f, 40.0f);

            ui_fontSizeSlider->setMinValue(11.0f);
            ui_fontSizeSlider->setMaxValue(32.0f);
            ui_fontSizeSlider->setValue(16.0f);

            ui_fontSizeSlider->stepMode = Slider::StepMode::Discrete;
            ui_fontSizeSlider->stepInterval = 1.0f;

            auto& valueLabelAppear = ui_fontSizeSlider->getAppearance().valueLabel;
            valueLabelAppear.offset = 14.0f;
            valueLabelAppear.mainRect.geometry.size = { 100.0f, 40.0f };
            ui_fontSizeSlider->loadValueLabelShadowBitmap();
            valueLabelAppear.isResident = true;

            ui_fontSizeSlider->valueLabel()->setTextFormat(D14_FONT(L"Default/Normal/16"));

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 1, 4 };
            geoInfo1.axis.y = { 1, 1 };
            ui_controlPanel->addElement(ui_fontSizeSlider, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Font size");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 0, 1 };
            geoInfo2.axis.y = { 1, 1 };
            geoInfo2.spacing.left = 24.0f;
            ui_controlPanel->addElement(ui_label, geoInfo2);
        }
        auto ui_fontWeightSelector = makeUIObject<HorzSlider>();
        auto wk_fontWeightSelector = (WeakPtr<HorzSlider>)ui_fontWeightSelector;
        {
            ui_fontWeightSelector->resize(180.0f, 40.0f);

            ui_fontWeightSelector->setMinValue(0.0f);
            ui_fontWeightSelector->setMaxValue(4.0f);
            ui_fontWeightSelector->setValue(2.0f);

            ui_fontWeightSelector->stepMode = Slider::StepMode::Discrete;
            ui_fontWeightSelector->stepInterval = 1.0f;

            auto& valueLabelAppear = ui_fontWeightSelector->getAppearance().valueLabel;
            valueLabelAppear.offset = 14.0f;
            valueLabelAppear.mainRect.geometry.size = { 120.0f, 40.0f };
            ui_fontWeightSelector->loadValueLabelShadowBitmap();
            valueLabelAppear.isResident = true;

            ui_fontWeightSelector->valueLabel()->setTextFormat(D14_FONT(L"Default/Normal/16"));

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 6, 4 };
            geoInfo1.axis.y = { 1, 1 };
            ui_controlPanel->addElement(ui_fontWeightSelector, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Font weight");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 5, 1 };
            geoInfo2.axis.y = { 1, 1 };
            ui_controlPanel->addElement(ui_label, geoInfo2);
        }

        // Set control-panel callbacks.

        auto changeTextBlockFont = [=](IDWriteTextFormat* textFormat)
        {
            if (!wk_textViewer.expired())
            {
                auto sh_textViewer = wk_textViewer.lock();
                if (sh_textViewer->currActiveCardTabIndex().valid())
                {
                    auto& tabItor = sh_textViewer->currActiveCardTabIndex().iterator;

                    if (tabItor->content->children().empty()) return;
                    auto layout = tabItor->content->children().begin();

                    if ((*layout)->children().empty()) return;
                    auto element = (*layout)->children().begin();

                    auto tblock = std::dynamic_pointer_cast<Label>(*element);
                    if (tblock != nullptr)
                    {
                        tblock->setTextFormat(textFormat);
                    }
                    (*layout)->resize(800.0f, tblock->textMetrics().height + 100.0f);

                    // Update the viewport offset of the scroll view to fix display bug.
                    auto content = std::dynamic_pointer_cast<ScrollView>(tabItor->content);
                    if (content != nullptr)
                    {
                        content->setViewportOffset(content->viewportOffset());
                    }
                }
            }
        };
        ui_fontNameSelector->f_onSelectedChange = [=](ComboBox* cb, IconLabel* content)
        {
            if (!wk_fontSizeSlider.expired() && !wk_fontWeightSelector.expired())
            {
                auto sh_fontSizeSlider = wk_fontSizeSlider.lock();
                auto sh_fontWeightSlider = wk_fontWeightSelector.lock();

                auto textFormatName =
                    content->label()->text() + L"/" +
                    sh_fontWeightSlider->valueLabel()->text() + L"/" +
                    std::to_wstring(math_utils::round(sh_fontSizeSlider->value()));

                changeTextBlockFont(textFormatMap->at(textFormatName).Get());
            }
        };
        ui_fontSizeSlider->f_onValueChange = [=](Slider::ValuefulObject* vobj, float value)
        {
            auto sldr = (Slider*)vobj;
            sldr->valueLabel()->setText(sldr->valueLabel()->text() + L" pt");

            if (!wk_fontNameSelector.expired() && !wk_fontWeightSelector.expired())
            {
                auto sh_fontNameSelector = wk_fontNameSelector.lock();
                auto sh_fontWeightSlider = wk_fontWeightSelector.lock();

                auto textFormatName =
                    sh_fontNameSelector->content()->label()->text() + L"/" +
                    sh_fontWeightSlider->valueLabel()->text() + L"/" +
                    std::to_wstring(math_utils::round(value));

                changeTextBlockFont(textFormatMap->at(textFormatName).Get());
            }
        };
        ui_fontWeightSelector->f_onValueChange = [=](Slider::ValuefulObject* vobj, float value)
        {
            auto sldr = (Slider*)vobj;
            sldr->valueLabel()->setText(fontWeightArray[math_utils::round(value)]);

            if (!wk_fontNameSelector.expired() && !wk_fontSizeSlider.expired())
            {
                auto sh_fontNameSelector = wk_fontNameSelector.lock();
                auto sh_fontSizeSlider = wk_fontSizeSlider.lock();

                auto textFormatName =
                    sh_fontNameSelector->content()->label()->text() + L"/" +
                    sldr->valueLabel()->text() + L"/" +
                    std::to_wstring(math_utils::round(sh_fontSizeSlider->value()));

                changeTextBlockFont(textFormatMap->at(textFormatName).Get());
            }
        };
        // We must update font-weight firstly since the callback of font-size
        // depends on the content of the value-label of the font-weight-panel.
        ui_fontWeightSelector->onValueChange(ui_fontWeightSelector->value());
        ui_fontSizeSlider->onValueChange(ui_fontSizeSlider->value());
    });
}
