#include "Common/Precompile.h"

#include <cstdlib>
#include <ctime>

#include "Common/DirectXError.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/CheckBox.h"
#include "UIKit/FileSystemUtils.h"
#include "UIKit/FilledButton.h"
#include "UIKit/GridLayout.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/RawTextBox.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/ScrollView.h"
#include "UIKit/TabCaption.h"
#include "UIKit/TabGroup.h"
#include "UIKit/TextBox.h"

using namespace d14engine;
using namespace d14engine::uikit;

#define D14_DEMO_NAME L"ImageViewer"

#define D14_MAINWINDOW_TITLE L"D14Engine - " D14_DEMO_NAME L" @ UIKit"
#define D14_SCREENSHOT_PATH L"Screenshots/" D14_DEMO_NAME L".png"

D14_SET_APP_ENTRY(mainImageViewer)
{
    srand((unsigned int)time(0));

    Application::CreateInfo info =
    {
        .windowSize = { 800, 600 }
    };
    return Application(info).run([](Application* app)
    {
        auto ui_mainWindow = makeRootUIObject<MainWindow>(D14_MAINWINDOW_TITLE);
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

            if (app->themeStyle().name == L"Light")
            {
                ui_darkModeSwitch->setOnOffState(OnOffSwitch::Off);
            }
            else ui_darkModeSwitch->setOnOffState(OnOffSwitch::On);

            app->f_onSystemThemeStyleChange = [app]
            (const Application::ThemeStyle& style)
            {
                app->setThemeStyle(style);
            };
            ui_darkModeSwitch->f_onStateChange = [app]
            (OnOffSwitch::StatefulObject* obj, OnOffSwitch::StatefulObject::Event& e)
            {
                Application::ThemeStyle style = app->themeStyle();
                if (e.on()) style.name = L"Dark";
                else if (e.off()) style.name = L"Light";
                app->setThemeStyle(style);
            };
        }
        auto ui_screenshot = makeRootUIObject<OutlinedButton>(L"Screenshot");
        {
            ui_screenshot->moveTopmost();
            ui_screenshot->transform(200.0f, 4.0f, 100.0f, 24.0f);
            ui_screenshot->content()->label()->setTextFormat(D14_FONT(L"Default/12"));

            ui_screenshot->f_onMouseButtonRelease = [app]
            (ClickablePanel* clkp, ClickablePanel::Event& e)
            {
                auto image = app->windowshot();
                CreateDirectory(L"Screenshots", nullptr);
                bitmap_utils::saveBitmap(image.Get(), D14_SCREENSHOT_PATH);
            };
        }
        auto ui_clientArea = makeUIObject<Panel>();
        {
            ui_mainWindow->setContent(ui_clientArea);
        }
        auto ui_tabGroup = makeManagedUIObject<TabGroup>(ui_clientArea);
        WeakPtr<TabGroup> wk_tabGroup = ui_tabGroup; // captured by lambdas
        {
            ui_tabGroup->setUIObjectPriority(0);
            ui_tabGroup->maximalWidthHint = 500.0f;
            ui_tabGroup->transform(0.0f, 40.0f, 500.0f, 524.0f);
            ui_tabGroup->isRightResizable = true;

            auto caption = makeUIObject<TabCaption>(L"Home");
            caption->title()->label()->setTextFormat(D14_FONT(L"Default/12"));

            auto content = makeUIObject<Label>(L"Try load a new image-tab...");
            THROW_IF_FAILED(content->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
            content->drawTextOptions = D2D1_DRAW_TEXT_OPTIONS_CLIP;

            ui_tabGroup->insertTab({ caption, content });
            ui_tabGroup->selectTab(0);
        }
        auto ui_sideLayout = makeManagedUIObject<GridLayout>(ui_clientArea);
        {
            ui_sideLayout->setUIObjectPriority(1);
            ui_sideLayout->transform(500.0f, 0.0f, 300.0f, 564.0f);
            ui_sideLayout->getAppearance().background.opacity = 1.0f;
            ui_sideLayout->setCellCount(5, 8);
        }
        auto ui_checkBox = makeUIObject<CheckBox>();
        {
            ui_checkBox->setChecked(CheckBox::Checked);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 0, 1 };
            ui_sideLayout->addElement(ui_checkBox, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Enable dynamic sizing");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 4 };
            geoInfo2.axis.y = { 0, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_checkBox->f_onStateChange = [=]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_tabGroup.expired())
                {
                    auto sh_tabGroup = wk_tabGroup.lock();
                    sh_tabGroup->enableDynamicSizing = e.checked();
                }
            };
        }
        Wstring assetsPath = L"Test/UIKit/ImageViewer/";

        std::vector<std::pair<Wstring, ComPtr<ID2D1Bitmap1>>> images;
        file_system_utils::foreachFileInDir(assetsPath, L"*.png", [&](WstrParam filePath)
        {
            auto fileName = file_system_utils::extractFileName(filePath);
            auto filePrefix = file_system_utils::extractFilePrefix(fileName);
            images.push_back({ filePrefix, bitmap_utils::loadBitmap(filePath) });
            return false;
        });
        auto ui_insertButton = makeUIObject<FilledButton>(L"Create new image tab");
        {
            ui_insertButton->roundRadiusX = ui_insertButton->roundRadiusY = 8.0f;
            if (images.empty()) ui_insertButton->setEnabled(false);
            ui_insertButton->resize(250.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 1, 1 };
            ui_sideLayout->addElement(ui_insertButton, geoInfo);

            ui_insertButton->f_onMouseButtonRelease = [=](ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (!wk_tabGroup.expired() && images.size() > 0)
                {
                    auto sh_tabGroup = wk_tabGroup.lock();
                    int index = rand() % (int)images.size();

                    auto caption = makeUIObject<TabCaption>(images[index].first);
                    caption->title()->label()->setTextFormat(D14_FONT(L"Default/12"));

                    auto imageRect = math_utils::sizeOnlyRect(images[index].second->GetSize());
                    auto content = makeUIObject<Panel>(imageRect, nullptr, images[index].second);
                    auto wrapper = makeUIObject<ScrollView>(content);

                    size_t currTabIndex = sh_tabGroup->currActiveCardTabIndex().index;
                    sh_tabGroup->insertTab({ caption, wrapper }, currTabIndex);
                    sh_tabGroup->selectTab(currTabIndex);

                    wrapper->f_onMouseWheel =
                    [wk_content = (WeakPtr<Panel>)content](Panel* p, MouseWheelEvent& e)
                    {
                        if (e.ALT() && !wk_content.expired())
                        {
                            auto sh_content = wk_content.lock();
                            auto bfsz = sh_content->size();
                            D2D1_SIZE_F afsz =
                            {
                                bfsz.width + 50.0f * e.deltaCount,
                                bfsz.height + 50.0f * e.deltaCount * bfsz.height / bfsz.width
                            };
                            if (afsz.width >= 50.0f && afsz.height >= 50.0f)
                            {
                                sh_content->resize(afsz);
                            }
                            auto sv = dynamic_cast<ScrollView*>(p);
                            if (sv != nullptr)
                            {
                                sv->setViewportOffset(sv->viewportOffset());
                            }
                        }
                    };
                }
            };
        }
        auto ui_textBox1 = makeUIObject<RawTextBox>(5.0f, math_utils::sizeOnlyRect({ 80.0f, 40.0f }));
        auto ui_textBox2 = makeUIObject<RawTextBox>(5.0f, math_utils::sizeOnlyRect({ 80.0f, 40.0f }));
        {
            ui_textBox1->setTextFormat(D14_FONT(L"Default/12"));
            ui_textBox1->setVisibleTextRect({ 5.0f, 8.0f, 75.0f, 32.0f });
            ui_textBox1->placeholder()->setText(L"first");
            ui_textBox1->placeholder()->setTextFormat(D14_FONT(L"Default/12"));

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 2 };
            geoInfo1.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_textBox1, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"~");
            THROW_IF_FAILED(ui_label->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 2, 1 };
            geoInfo2.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_textBox2->setTextFormat(D14_FONT(L"Default/12"));
            ui_textBox2->setVisibleTextRect({ 5.0f, 8.0f, 75.0f, 32.0f });
            ui_textBox2->placeholder()->setText(L"last");
            ui_textBox2->placeholder()->setTextFormat(D14_FONT(L"Default/12"));

            GridLayout::GeometryInfo geoInfo3 = {};
            geoInfo3.isFixedSize = true;
            geoInfo3.axis.x = { 3, 2 };
            geoInfo3.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_textBox2, geoInfo3);
        }
        auto ui_removeButton = makeUIObject<FilledButton>(L"Remove specified range");
        {
            ui_removeButton->roundRadiusX = ui_removeButton->roundRadiusY = 8.0f;
            ui_removeButton->resize(250.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 3, 1 };
            ui_sideLayout->addElement(ui_removeButton, geoInfo);

            ui_removeButton->f_onMouseButtonRelease =
            [=,
                wk_textBox1 = (WeakPtr<RawTextBox>)ui_textBox1,
                wk_textBox2 = (WeakPtr<RawTextBox>)ui_textBox2
            ]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (!wk_tabGroup.expired() && !wk_textBox1.expired() && !wk_textBox2.expired())
                {
                    auto sh_tabGroup = wk_tabGroup.lock();
                    auto sh_textBox1 = wk_textBox1.lock();
                    auto sh_textBox2 = wk_textBox2.lock();

                    auto first = (size_t)_wtoi(sh_textBox1->text().c_str());
                    auto last = std::max(first, (size_t)_wtoi(sh_textBox2->text().c_str()));

                    sh_tabGroup->removeTab(first, last - first + 1);

                    sh_textBox1->setText(std::to_wstring(first));
                    sh_textBox2->setText(std::to_wstring(last));
                }
            };
        }
        auto ui_titleEditor = makeUIObject<TextBox>(5.0f, math_utils::sizeOnlyRect({ 256.0f, 46.0f }));
        {
            ui_titleEditor->setVisibleTextRect({ 5.0f, 8.0f, 251.0f, 38.0f });
            ui_titleEditor->placeholder()->setText(L"Edit selected tab's title");

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 4, 1 };
            ui_sideLayout->addElement(ui_titleEditor, geoInfo);

            ui_titleEditor->f_onTextChange = [=](RawTextBox::TextInputObject* obj, WstrParam text)
            {
                if (!wk_tabGroup.expired())
                {
                    auto& selected = wk_tabGroup.lock()->currActiveCardTabIndex();
                    if (selected.valid()) selected->caption->title()->label()->setText(text);
                }
            };
        }
        auto ui_checkBox1 = makeUIObject<CheckBox>();
        {
            ui_checkBox1->setCheckedState(CheckBox::Checked);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 5, 1 };
            ui_sideLayout->addElement(ui_checkBox1, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Closable ('X' button)");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 4 };
            geoInfo2.axis.y = { 5, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_checkBox1->f_onStateChange = [=]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_tabGroup.expired())
                {
                    auto& selected = wk_tabGroup.lock()->currActiveCardTabIndex();
                    if (selected.valid()) selected->caption->closable = e.checked();
                }
            };
        }
        auto ui_checkBox2 = makeUIObject<CheckBox>();
        {
            ui_checkBox2->setCheckedState(CheckBox::Checked);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 6, 1 };
            ui_sideLayout->addElement(ui_checkBox2, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Draggable (shift tab)");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 4 };
            geoInfo2.axis.y = { 6, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_checkBox2->f_onStateChange = [=]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_tabGroup.expired())
                {
                    auto& selected = wk_tabGroup.lock()->currActiveCardTabIndex();
                    if (selected.valid()) selected->caption->draggable = e.checked();
                }
            };
        }
        auto ui_checkBox3 = makeUIObject<CheckBox>();
        {
            ui_checkBox3->setCheckedState(CheckBox::Unchecked);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 7, 1 };
            ui_sideLayout->addElement(ui_checkBox3, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Promotable (window)");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 4 };
            geoInfo2.axis.y = { 7, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_checkBox3->f_onStateChange = [=]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_tabGroup.expired())
                {
                    auto& selected = wk_tabGroup.lock()->currActiveCardTabIndex();
                    if (selected.valid()) selected->caption->promotable = e.checked();
                }
            };
        }
        // Update tab-property-inspectors dynamically.
        {
            ui_tabGroup->f_onSelectedTabIndexChange =
            [
                wk_titleEditor = (WeakPtr<TextBox>)ui_titleEditor,

                wk_checkBox1 = (WeakPtr<CheckBox>)ui_checkBox1,
                wk_checkBox2 = (WeakPtr<CheckBox>)ui_checkBox2,
                wk_checkBox3 = (WeakPtr<CheckBox>)ui_checkBox3
            ]
            (TabGroup* tg, TabGroup::TabIndexParam index)
            {
                if (!wk_titleEditor.expired())
                {
                    wk_titleEditor.lock()->setText(index.valid() ?
                        index->caption->title()->label()->text() : L"");
                }
                // We should use setCheckedState instead of setChecked here:
                //
                // 1. setChecked:
                //
                //    Change state && trigger related callback.
                //
                // 2. setCheckedState:
                //
                //    Only change state, no callback triggered.

                if (!wk_checkBox1.expired())
                {
                    wk_checkBox1.lock()->setCheckedState(
                        index.valid() && index->caption->closable ?
                        CheckBox::Checked :
                        CheckBox::Unchecked);
                }
                if (!wk_checkBox2.expired())
                {
                    wk_checkBox2.lock()->setCheckedState(
                        index.valid() && index->caption->draggable ?
                        CheckBox::Checked :
                        CheckBox::Unchecked);
                }
                if (!wk_checkBox3.expired())
                {
                    wk_checkBox3.lock()->setCheckedState(
                        index.valid() && index->caption->promotable ?
                        CheckBox::Checked :
                        CheckBox::Unchecked);
                }
            };
            ui_tabGroup->f_onTriggerTabPromoting = [](TabGroup* tg, Window* w)
            {
                w->registerDrawObjects();
                w->registerApplicationEvents();

                w->moveTopmost();

                w->isMinimizeEnabled = false;
                w->isMaximizeEnabled = false;

                w->f_onClose = [](Window* w) { w->release(); };
            };
        }
    });
}
