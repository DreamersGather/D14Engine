#include "Common/Precompile.h"

#include "Renderer/Renderer.h"
#include "Renderer/TickTimer.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/FanimPanel.h"
#include "UIKit/FileSystemUtils.h"
#include "UIKit/GridLayout.h"
#include "UIKit/HorzSlider.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/RawTextBox.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/TabCaption.h"
#include "UIKit/TabGroup.h"
#include "UIKit/ToggleButton.h"

using namespace d14engine;
using namespace d14engine::renderer;
using namespace d14engine::uikit;

D14_SET_APP_ENTRY(mainPixelAnimation)
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
        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - PixelAnimation @ UIKit");
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

            ui_screenshot->f_onMouseButtonRelease = [=]
            (ClickablePanel* clkp, ClickablePanel::Event& e)
            {
                auto image = app->screenshot();
                CreateDirectory(L"Screenshots", nullptr);
                bitmap_utils::saveBitmap(image.Get(), L"Screenshots/DemoTemplate.png");
            };
        }
        auto ui_clientArea = makeUIObject<Panel>();
        {
            ui_mainWindow->setCenterUIObject(ui_clientArea);
        }
        auto ui_pixelViewer = makeManagedUIObject<TabGroup>(ui_clientArea);
        auto wk_pixelViewer = (WeakPtr<TabGroup>)ui_pixelViewer;
        {
            ui_pixelViewer->transform(0.0f, 40.0f, 500.0f, 524.0f);
        }
        auto rawFrames = std::make_shared<animation_utils::FramePackage>();
        auto ui_stickBoy = makeUIObject<FanimPanel>();
        auto wk_stickBoy = (WeakPtr<FanimPanel>)ui_stickBoy;
        {
            ui_stickBoy->resize(256.0f, 256.0f);

#ifdef _DEBUG
            Wstring assetsPath = L"Test/UIKit/PixelAnimation/stick_boy/";
#else
            Wstring assetsPath = L"stick_boy/"; // same path with the executable
#endif
            file_system_utils::foreachFileInDir(assetsPath, L"*.png", [&](WstrParam& path)
            {
                auto name = file_system_utils::extractFilePrefix(
                            file_system_utils::extractFileName(path));

                BitmapObject bmpobj = bitmap_utils::loadBitmap(path);
                bmpobj.interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

                rawFrames->insert({ name, bmpobj });

                return false;
            });
            auto& frames = ui_stickBoy->bitmapData.fanim.frames;
            frames.resize(rawFrames->size());
            for (auto& kv : *rawFrames)
            {
                ui_stickBoy->bitmapData.fanim.frames[std::stoi(kv.first)] = kv.second;
                ui_stickBoy->bitmapData.fanim.timeSpanDataInSecs = 0.06f;
            }
            auto caption = makeUIObject<TabCaption>(L"stick_boy");
            caption->title()->label()->setTextFormat(D14_FONT(L"Default/Normal/12"));

            auto content = makeUIObject<GridLayout>();
            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 1 };
            geoInfo.axis.y = { 0, 1 };
            content->addElement(ui_stickBoy, geoInfo);

            ui_pixelViewer->appendTab({ caption, content });
            ui_pixelViewer->selectTab(0);

            for (int i = 0; i < frames.size(); ++i)
            {
                auto ui_stickBoyFrame = makeUIObject<Panel>();
                ui_stickBoyFrame->resize(256.0f, 256.0f);
                ui_stickBoyFrame->bitmap = frames[i].bitmap;
                auto& interpMode = ui_stickBoyFrame->bitmapProperty.interpolationMode;
                interpMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

                auto caption = makeUIObject<TabCaption>(std::to_wstring(i));
                caption->title()->label()->setTextFormat(D14_FONT(L"Default/Normal/12"));

                auto content = makeUIObject<GridLayout>();
                GridLayout::GeometryInfo geoInfo = {};
                geoInfo.isFixedSize = true;
                geoInfo.axis.x = { 0, 1 };
                geoInfo.axis.y = { 0, 1 };
                content->addElement(ui_stickBoyFrame, geoInfo);

                ui_pixelViewer->appendTab({ caption, content });
            }
        }
        auto ui_sideLayout = makeManagedUIObject<GridLayout>(ui_clientArea);
        {
            ui_sideLayout->transform(500.0f, 0.0f, 300.0f, 564.0f);
            ui_sideLayout->setCellCount(5, 8);
        }
        auto ui_fpsLabel = makeUIObject<Label>(L"FPS: None");
        auto wk_fpsLabel = (WeakPtr<Label>)ui_fpsLabel;
        auto ui_animCtrlButton = makeUIObject<ToggleButton>(L"Start Anim");
        auto wk_animCtrlButton = (WeakPtr<ToggleButton>)ui_animCtrlButton;
        {
            ui_fpsLabel->resize(250.0f, 50.0f);

            ui_animCtrlButton->roundRadiusX =
            ui_animCtrlButton->roundRadiusY = 8.0f;
            ui_animCtrlButton->resize(250.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 5 };
            geoInfo1.axis.y = { 0, 1 };
            ui_sideLayout->addElement(ui_fpsLabel, geoInfo1);

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = true;
            geoInfo2.axis.x = { 0, 5 };
            geoInfo2.axis.y = { 1, 1 };
            ui_sideLayout->addElement(ui_animCtrlButton, geoInfo2);

            ui_fpsLabel->f_onRendererUpdateObject2DAfter = [=](Panel* p, Renderer* rndr)
            {
                static UINT fps = 0;
                if (!wk_animCtrlButton.expired())
                {
                    if (wk_animCtrlButton.lock()->currState().activated())
                    {
                        if (rndr->timer()->fps() != fps)
                        {
                            fps = rndr->timer()->fps();
                            ((Label*)p)->setText(L"FPS: " + std::to_wstring(fps));
                        }
                    }
                }
            };
            ui_animCtrlButton->f_onStateChange = [=]
            (ToggleButton::StatefulObject* obj, ToggleButton::StatefulObject::Event& e)
            {
                if (!wk_fpsLabel.expired() && !wk_stickBoy.expired())
                {
                    auto sh_stickBoy = wk_stickBoy.lock();
                    auto sh_animCtrlButton = wk_animCtrlButton.lock();
                    if (e.activated())
                    {
                        sh_stickBoy->increaseAnimationCount();
                        sh_animCtrlButton->content()->label()->setText(L"Stop Anim");
                    }
                    else if (e.deactivated())
                    {
                        sh_stickBoy->decreaseAnimationCount();
                        sh_animCtrlButton->content()->label()->setText(L"Start Anim");
                        wk_fpsLabel.lock()->setText(L"FPS: None");
                    }
                }
            };
        }
        auto ui_timeSpanLabel = makeUIObject<Label>(L"Time Span");
        auto ui_timeSpanInput = makeUIObject<RawTextBox>(5.0f, math_utils::sizeOnlyRect({ 120.0f, 42.0f }));
        auto wk_timeSpanInput = (WeakPtr<RawTextBox>)ui_timeSpanInput;
        {
            ui_timeSpanLabel->resize(250.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 5 };
            geoInfo1.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_timeSpanLabel, geoInfo1);

            ui_timeSpanInput->moveTopmost();
            ui_timeSpanInput->setText(L"0.06 s");
            ui_timeSpanInput->setVisibleTextRect({ 5.0f, 8.0f, 115.0f, 34.0f });

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = true;
            geoInfo2.axis.x = { 2, 3 };
            geoInfo2.axis.y = { 2, 1 };
            ui_sideLayout->addElement(ui_timeSpanInput, geoInfo2);

            ui_timeSpanInput->f_onLoseFocus = [=](Panel* p)
            {
                if (!wk_stickBoy.expired())
                {
                    auto pp = (RawTextBox*)p;
                    auto& fanim = wk_stickBoy.lock()->bitmapData.fanim;
                    try
                    {
                        std::wstringstream ss;
                        float timeSpanInSecs = std::stof(pp->text());
                        ss << std::fixed << std::setprecision(2) << timeSpanInSecs;

                        pp->setText(ss.str() + L" s");
                        fanim.timeSpanDataInSecs = timeSpanInSecs;
                    }
                    catch (...) // std::stof failed
                    {
                        pp->setText(L"0.06 s");
                        fanim.timeSpanDataInSecs = 0.06f;
                    }
                }
            };
        }
        ui_stickBoy->f_onDestroy = [=](Panel* p)
        {
            // No need to do the clearing if the application already destroyed.
            if (!Application::g_app) return;

            if (p->isPlayAnimation()) p->decreaseAnimationCount();

            if (!wk_fpsLabel.expired())
            {
                wk_fpsLabel.lock()->setText(L"FPS: None");
            }
            if (!wk_animCtrlButton.expired())
            {
                auto sh_animCtrlButton = wk_animCtrlButton.lock();
                sh_animCtrlButton->setEnabled(false);
                sh_animCtrlButton->setActivatedState(ToggleButton::DEACTIVATED);
            }
            if (!wk_timeSpanInput.expired())
            {
                auto sh_timeSpanInput = wk_timeSpanInput.lock();
                sh_timeSpanInput->setEnabled(false);
                sh_timeSpanInput->LabelArea::setText(L"0.06 s");
            }
        };
        auto ui_frameSizeLabel = makeUIObject<Label>(L"Frame Size (in pixel)");
        auto ui_frameSizeSlider = makeUIObject<HorzSlider>();
        auto wk_frameSizeSlider = (WeakPtr<HorzSlider>)ui_frameSizeSlider;
        {
            ui_frameSizeLabel->resize(250.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 5 };
            geoInfo1.axis.y = { 3, 1 };
            ui_sideLayout->addElement(ui_frameSizeLabel, geoInfo1);

            ui_frameSizeSlider->resize(250.0f, 50.0f);

            ui_frameSizeSlider->setMinValue(64.0f);
            ui_frameSizeSlider->setMaxValue(512.0f);
            ui_frameSizeSlider->setValue(256.0f);

            ui_frameSizeSlider->stepMode = Slider::StepMode::Discrete;
            ui_frameSizeSlider->stepInterval = 16.0f;

            auto& valueLabelAppear = ui_frameSizeSlider->getAppearance().valueLabel;
            valueLabelAppear.isResident = true;

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = true;
            geoInfo2.axis.x = { 0, 5 };
            geoInfo2.axis.y = { 4, 1 };
            ui_sideLayout->addElement(ui_frameSizeSlider, geoInfo2);

            ui_frameSizeSlider->f_onValueChange = [=]
            (Slider::ValuefulObject* vobj, float value)
            {
                if (!wk_pixelViewer.expired())
                {
                    auto& selected = wk_pixelViewer.lock()->currActiveCardTabIndex();
                    if (selected.valid())
                    {
                        auto layout = std::dynamic_pointer_cast<GridLayout>(selected->content);
                        if (layout)
                        {
                            auto itor = layout->children().begin();
                            if (itor != layout->children().end())
                            {
                                (*itor)->resize(value, value);
                                layout->updateAllElements();
                            }
                        }
                    }
                }
            };
            ui_pixelViewer->f_onSelectedTabIndexChange = [=]
            (TabGroup * tg, TabGroup::TabIndexParam index)
            {
                if (index.valid() && !wk_frameSizeSlider.expired())
                {
                    auto layout = std::dynamic_pointer_cast<GridLayout>(index->content);
                    if (layout)
                    {
                        auto itor = layout->children().begin();
                        if (itor != layout->children().end())
                        {
                            wk_frameSizeSlider.lock()->setValue((*itor)->width());
                        }
                    }
                }
            };
        }
    });
}
