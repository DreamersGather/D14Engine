#include "Common/Precompile.h"

#include "Common/DirectXError.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/CheckBox.h"
#include "UIKit/FilledButton.h"
#include "UIKit/GridLayout.h"
#include "UIKit/HorzSlider.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/ListView.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/TreeView.h"

using namespace d14engine;
using namespace d14engine::uikit;

#define D14_DEMO_NAME L"ListTreeView"

#define D14_MAINWINDOW_TITLE L"D14Engine - " D14_DEMO_NAME L" @ UIKit"
#define D14_SCREENSHOT_PATH L"Screenshots/" D14_DEMO_NAME L".png"

D14_SET_APP_ENTRY(mainListTreeView)
{
    Application::CreateInfo info =
    {
        .windowSize = { 800, 600 }
    };
    return Application(info).run([&](Application* app)
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
        auto ui_listView = makeManagedUIObject<ListView>(ui_clientArea);
        {
            ui_listView->transform(0.0f, 0.0f, 400.0f, 282.0f);

            ui_listView->getAppearance().background.opacity = 1.0f;
            ui_listView->getAppearance().stroke.opacity = 1.0f;

            // Populate list-view items.
            ListView::ItemList ui_listViewItemList = {};
            for (int i = 1; i <= 30; ++i)
            {
                ui_listViewItemList.push_back(makeUIObject<ListViewItem>(
                    std::to_wstring(i), math_utils::heightOnlyRect(30.0f)));
            }
            ui_listView->appendItem(ui_listViewItemList);
        }
        auto ui_treeView = makeManagedUIObject<TreeView>(ui_clientArea);
        {
            ui_treeView->transform(0.0f, 282.0f, 400.0f, 282.0f);

            ui_treeView->getAppearance().background.opacity = 1.0f;
            ui_treeView->getAppearance().stroke.opacity = 1.0f;

            // Populate tree-view items.
            TreeView::ItemList ui_treeViewItemList1 = {};
            for (int i = 1; i <= 3; ++i)
            {
                auto ui_treeViewItem1 = makeUIObject<TreeViewItem>(
                    L"1 - " + std::to_wstring(i), math_utils::heightOnlyRect(30.0f));

                TreeView::ItemList ui_treeViewItemList2 = {};
                for (int j = 1; j <= 4; ++j)
                {
                    auto ui_treeViewItem2 = makeUIObject<TreeViewItem>(
                        L"2 - " + std::to_wstring(j), math_utils::heightOnlyRect(30.0f));

                    TreeView::ItemList ui_treeViewItemList3 = {};
                    for (int k = 1; k <= 5; ++k)
                    {
                        ui_treeViewItemList3.push_back(makeUIObject<TreeViewItem>(
                            L"3 - " + std::to_wstring(k), math_utils::heightOnlyRect(30.0f)));
                    }
                    ui_treeViewItem2->appendItem(ui_treeViewItemList3);
                    ui_treeViewItem2->setFolded(TreeViewItem::FOLDED);
                    ui_treeViewItemList2.push_back(ui_treeViewItem2);
                }
                ui_treeViewItem1->appendItem(ui_treeViewItemList2);
                ui_treeViewItem1->setFolded(TreeViewItem::FOLDED);
                ui_treeViewItemList1.push_back(ui_treeViewItem1);
            }
            ui_treeView->appendRootItem(ui_treeViewItemList1);
        }
        auto ui_sideLayout = makeManagedUIObject<GridLayout>(ui_clientArea);
        {
            ui_sideLayout->transform(400.0f, 0.0f, 400.0f, 564.0f);
            ui_sideLayout->setCellCount(5, 10);
        }
        auto ui_banner1 = makeUIObject<Label>(L"------- List View -------");
        {
            THROW_IF_FAILED(ui_banner1->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = false;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 0, 1 };
            ui_sideLayout->addElement(ui_banner1, geoInfo);
        }
        auto ui_checkBox1 = makeUIObject<CheckBox>(true);
        {
            ui_checkBox1->setCheckedState(CheckBox::Checked);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 1, 1 };
            ui_sideLayout->addElement(ui_checkBox1, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Select Mode: Extended");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 4 };
            geoInfo2.axis.y = { 1, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_checkBox1->f_onStateChange =
            [
                wk_listView = (WeakPtr<ListView>)ui_listView,
                wk_label = (WeakPtr<Label>)ui_label
            ]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_listView.expired() && !wk_label.expired())
                {
                    auto sd_listView = wk_listView.lock();
                    auto sd_label = wk_label.lock();
                    if (e.unchecked())
                    {
                        sd_listView->selectMode = ListView::SelectMode::Single;
                        sd_label->setText(L"Select Mode: Single");
                    }
                    else if (e.intermediate())
                    {
                        sd_listView->selectMode = ListView::SelectMode::Multiple;
                        sd_label->setText(L"Select Mode: Multiple");
                    }
                    else if (e.checked())
                    {
                        sd_listView->selectMode = ListView::SelectMode::Extended;
                        sd_label->setText(L"Select Mode: Extended");
                    }
                }
            };
        }
        auto ui_insertButton1 = makeUIObject<FilledButton>(
            L"Insert before selected", 5.0f, math_utils::sizeOnlyRect({ 300.0f, 40.0f }));
        {
            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_insertButton1, geoInfo);

            ui_insertButton1->f_onMouseButtonRelease =
            [
                wk_listView = (WeakPtr<ListView>)ui_listView
            ]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (!wk_listView.expired())
                {
                    auto sd_listView = wk_listView.lock();
                    if (!sd_listView->selectedItemIndices().empty())
                    {
                        auto selected = sd_listView->selectedItemIndices().begin();
                        auto selectedContent = (*selected->iterator)->getContent<IconLabel>().lock();
                        sd_listView->insertItem(
                        {
                            makeUIObject<ListViewItem>(
                                selectedContent->label()->text() + L"_new",
                                math_utils::heightOnlyRect(30.0f))
                        },
                        selected->index);
                    }
                }
            };
        }
        auto ui_removeButton1 = makeUIObject<FilledButton>(
            L"Remove current selected", 5.0f, math_utils::sizeOnlyRect({ 300.0f, 40.0f }));
        {
            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 3, 1 };
            ui_sideLayout->addElement(ui_removeButton1, geoInfo);

            ui_removeButton1->f_onMouseButtonRelease =
            [
                wk_listView = (WeakPtr<ListView>)ui_listView
            ]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (!wk_listView.expired())
                {
                    auto sd_listView = wk_listView.lock();
                    while (!sd_listView->selectedItemIndices().empty())
                    {
                        (*sd_listView->selectedItemIndices().begin()->iterator)->release();
                    }
                }
            };
        }
        auto ui_banner2 = makeUIObject<Label>(L"------- Tree View -------");
        {
            THROW_IF_FAILED(ui_banner2->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = false;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 4, 1 };
            ui_sideLayout->addElement(ui_banner2, geoInfo);
        }
        auto ui_checkBox2 = makeUIObject<CheckBox>(true);
        {
            ui_checkBox2->setCheckedState(CheckBox::Checked);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 5, 1 };
            ui_sideLayout->addElement(ui_checkBox2, geoInfo1);

            auto ui_label = makeUIObject<Label>(L"Select Mode: Extended");

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = false;
            geoInfo2.axis.x = { 1, 4 };
            geoInfo2.axis.y = { 5, 1 };
            ui_sideLayout->addElement(ui_label, geoInfo2);

            ui_checkBox2->f_onStateChange =
            [
                wk_treeView = (WeakPtr<TreeView>)ui_treeView,
                wk_label = (WeakPtr<Label>)ui_label
            ]
            (CheckBox::StatefulObject* obj, CheckBox::StatefulObject::Event& e)
            {
                if (!wk_treeView.expired())
                {
                    auto sd_treeView = wk_treeView.lock();
                    auto sd_label = wk_label.lock();
                    if (e.unchecked())
                    {
                        sd_treeView->selectMode = TreeView::SelectMode::Single;
                        sd_label->setText(L"Select Mode: Single");
                    }
                    else if (e.intermediate())
                    {
                        sd_treeView->selectMode = TreeView::SelectMode::Multiple;
                        sd_label->setText(L"Select Mode: Multiple");
                    }
                    else if (e.checked())
                    {
                        sd_treeView->selectMode = TreeView::SelectMode::Extended;
                        sd_label->setText(L"Select Mode: Extended");
                    }
                }
            };
        }
        auto ui_insertButton2 = makeUIObject<FilledButton>(
            L"Insert child into selected", 5.0f, math_utils::sizeOnlyRect({ 300.0f, 40.0f }));
        {
            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 6, 1 };
            ui_sideLayout->addElement(ui_insertButton2, geoInfo);

            ui_insertButton2->f_onMouseButtonRelease =
            [
                wk_treeView = (WeakPtr<TreeView>)ui_treeView
            ]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (!wk_treeView.expired())
                {
                    auto sd_treeView = wk_treeView.lock();
                    if (!sd_treeView->selectedItemIndices().empty())
                    {
                        auto selected = sd_treeView->selectedItemIndices().begin();
                        auto selectedContent = (*selected->iterator)->getContent<IconLabel>().lock();
                        (*selected->iterator)->insertItem(
                        {
                            makeUIObject<TreeViewItem>(
                                selectedContent->label()->text() + L"_child",
                                math_utils::heightOnlyRect(30.0f))
                        });
                    }
                }
            };
        }
        auto ui_insertButton3 = makeUIObject<FilledButton>(
            L"Insert peer before selected", 5.0f, math_utils::sizeOnlyRect({ 300.0f, 40.0f }));
        {
            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 7, 1 };
            ui_sideLayout->addElement(ui_insertButton3, geoInfo);

            ui_insertButton3->f_onMouseButtonRelease =
            [
                wk_treeView = (WeakPtr<TreeView>)ui_treeView
            ]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                // Inserting root-peer and inserting child-peer are different!
                if (!wk_treeView.expired())
                {
                    auto sd_treeView = wk_treeView.lock();
                    if (!sd_treeView->selectedItemIndices().empty())
                    {
                        auto selected = sd_treeView->selectedItemIndices().begin();
                        if ((*selected->iterator)->parentItem().expired()) // Insert as a root-peer.
                        {
                            size_t index = 0;
                            for (auto& item : sd_treeView->rootItems())
                            {
                                if (cpp_lang_utils::isMostDerivedEqual(item, *selected->iterator))
                                {
                                    auto selectedContent = (*selected->iterator)->getContent<IconLabel>().lock();
                                    sd_treeView->insertRootItem(
                                    {
                                        makeUIObject<TreeViewItem>(
                                            selectedContent->label()->text() + L"_peer",
                                            math_utils::heightOnlyRect(30.0f))
                                    },
                                    index); break;
                                }
                                ++index;
                            }
                        }
                        else // The selected is managed by another item, so insert as a child-peer.
                        {
                            size_t index = 0;
                            auto parentItem = (*selected->iterator)->parentItem().lock();
                            for (auto& item : parentItem->childrenItems())
                            {
                                if (cpp_lang_utils::isMostDerivedEqual(item.ptr, *selected->iterator))
                                {
                                    auto selectedContent = (*selected->iterator)->getContent<IconLabel>().lock();
                                    parentItem->insertItem(
                                    {
                                        makeUIObject<TreeViewItem>(
                                            selectedContent->label()->text() + L"_peer",
                                            math_utils::heightOnlyRect(30.0f))
                                    },
                                    index); break;
                                }
                                ++index;
                            }
                        }
                    }
                }
            };
        }
        auto ui_removeButton2 = makeUIObject<FilledButton>(
            L"Remove current selected", 5.0f, math_utils::sizeOnlyRect({ 300.0f, 40.0f }));
        {
            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 5 };
            geoInfo.axis.y = { 8, 1 };
            ui_sideLayout->addElement(ui_removeButton2, geoInfo);

            ui_removeButton2->f_onMouseButtonRelease =
            [
                wk_treeView = (WeakPtr<TreeView>)ui_treeView
            ]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (!wk_treeView.expired())
                {
                    auto sd_treeView = wk_treeView.lock();
                    while (!sd_treeView->selectedItemIndices().empty())
                    {
                        (*sd_treeView->selectedItemIndices().begin()->iterator)->release();
                    }
                }
            };
        }
        auto ui_slider = makeUIObject<HorzSlider>(math_utils::sizeOnlyRect({ 120.0f, 40.0f }));
        {
            auto ui_label = makeUIObject<Label>(L"Change selected height");

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = false;
            geoInfo1.axis.x = { 0, 3 };
            geoInfo1.axis.y = { 9, 1 };
            geoInfo1.spacing.left = 18.0f;
            geoInfo1.spacing.bottom = 6.0f;
            ui_sideLayout->addElement(ui_label, geoInfo1);

            ui_slider->moveTopmost();
            ui_slider->setMinValue(30.0f);
            ui_slider->setMaxValue(90.0f);
            ui_slider->setValue(30.0f);

            ui_slider->stepMode = Slider::StepMode::Discrete;
            ui_slider->stepInterval = 1.0f;

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = true;
            geoInfo2.axis.x = { 3, 2 };
            geoInfo2.axis.y = { 9, 1 };
            ui_sideLayout->addElement(ui_slider, geoInfo2);

            ui_slider->f_onValueChange =
            [
                wk_listView = (WeakPtr<ListView>)ui_listView,
                wk_treeView = (WeakPtr<TreeView>)ui_treeView
            ]
            (Slider::ValuefulObject* obj, float value)
            {
                if (!wk_listView.expired())
                {
                    auto sh_listView = wk_listView.lock();
                    for (auto& selected : sh_listView->selectedItemIndices())
                    {
                        auto& item = *selected.iterator;
                        item->resize(item->width(), value);
                    }
                    sh_listView->updateItemConstraints();
                    sh_listView->updateItemIndexRangeActivity();
                }
                if (!wk_treeView.expired())
                {
                    auto sh_treeView = wk_treeView.lock();
                    for (auto& selected : sh_treeView->selectedItemIndices())
                    {
                        auto& item = *selected.iterator;
                        if (item->peekItemImpl() != nullptr)
                        {
                            item->peekItemImpl()->setUnfoldedHeight(value);
                        }
                        else item->resize(item->width(), value);
                    }
                    sh_treeView->updateItemConstraints();
                    sh_treeView->updateItemIndexRangeActivity();
                }
            };
        }
    });
}
