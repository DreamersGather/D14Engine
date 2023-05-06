#include "Common/Precompile.h"

#include "Common/DirectXError.h"

#include "Renderer/Renderer.h"
#include "Renderer/TickTimer.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/Button.h"
#include "UIKit/Cursor.h"
#include "UIKit/ElevatedButton.h"
#include "UIKit/FilledButton.h"
#include "UIKit/FlatButton.h"
#include "UIKit/GridLayout.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/ToggleButton.h"

using namespace d14engine;
using namespace d14engine::renderer;
using namespace d14engine::uikit;

D14_SET_APP_ENTRY(mainButtonFamily)
{
    Application::CreateInfo info = {};
    if (argc >= 2 && lstrcmp(argv[1], L"HighDPI") == 0)
    {
        info.dpi = 192.0f;
    }
    else info.dpi = 96.0f;
    info.win32WindowRect = { 0, 0, 800, 600 };

    return Application(argc, argv, info).run([&](Application* app)
    {
        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - ButtonFamily @ UIKit");
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

            ui_screenshot->f_onMouseButtonRelease = []
            (ClickablePanel* clkp, ClickablePanel::Event& e)
            {
                auto image = Application::g_app->screenshot();
                CreateDirectory(L"Screenshots", nullptr);
                bitmap_utils::saveBitmap(image.Get(), L"Screenshots/ButtonFamily.png");
            };
        }
        auto ui_centerLayout = makeUIObject<GridLayout>();
        {
            ui_centerLayout->getAppearance().background.opacity = 1.0f;
            ui_centerLayout->setCellCount(2, 3);

            ui_mainWindow->setCenterUIObject(ui_centerLayout);
        }
        auto ui_button = makeUIObject<Button>(L"Change click count");
        {
            ui_button->resize(200.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 0, 1 };
            geoInfo.axis.y = { 0, 1 };
            ui_centerLayout->addElement(ui_button, geoInfo);

            auto ui_label1 = makeManagedUIObject<Label>(ui_centerLayout, L"Current count: 0");
            ui_label1->transform(math_utils::offset(ui_button->relativeRect(), { 0.0f, -60.0f }));
            THROW_IF_FAILED(ui_label1->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            auto ui_label2 = makeManagedUIObject<Label>(ui_centerLayout);
            ui_label2->transform(math_utils::offset(ui_button->relativeRect(), { 0.0f, 60.0f }));
            ui_label2->setText(L"Left ++ , Right --\nMiddle 0 , Wheel Change");
            THROW_IF_FAILED(ui_label2->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            auto currCount = std::make_shared<int>(0);
            auto wk_label1 = (WeakPtr<Label>)ui_label1;

            ui_button->f_onMouseButtonRelease = [=]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                if (e.left()) ++(*currCount);
                else if (e.right()) --(*currCount);
                else if (e.middle()) (*currCount) = 0;

                if (!wk_label1.expired())
                {
                    auto sh_label1 = wk_label1.lock();
                    sh_label1->setText(L"Current count: " + std::to_wstring(*currCount));
                }
            };
            ui_button->f_onMouseWheel = [=](Panel* p, MouseWheelEvent& e)
            {
                (*currCount) += e.deltaCount;

                if (!wk_label1.expired())
                {
                    auto sh_label1 = wk_label1.lock();
                    sh_label1->setText(L"Current count: " + std::to_wstring(*currCount));
                }
            };
        }
        auto ui_toggleButton = makeUIObject<ToggleButton>(L"Play Animation");
        {
            ui_toggleButton->resize(200.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo = {};
            geoInfo.isFixedSize = true;
            geoInfo.axis.x = { 1, 1 };
            geoInfo.axis.y = { 0, 1 };
            ui_centerLayout->addElement(ui_toggleButton, geoInfo);

            auto ui_label1 = makeManagedUIObject<Label>(ui_centerLayout, L"Animation Mode: Off");
            ui_label1->transform(math_utils::offset(ui_toggleButton->relativeRect(), { 0.0f, -60.0f }));
            THROW_IF_FAILED(ui_label1->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            auto ui_label2 = makeManagedUIObject<Label>(ui_centerLayout, L"Current cursor: Arrow");
            ui_label2->transform(math_utils::offset(ui_toggleButton->relativeRect(), { 0.0f, 60.0f }));
            THROW_IF_FAILED(ui_label2->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

            auto useDynamicCursor = std::make_shared<bool>(false);
            auto wk_label1 = (WeakPtr<Label>)ui_label1;
            auto wk_label2 = (WeakPtr<Label>)ui_label2;

            ui_centerLayout->f_onMouseMove = [=](Panel* p, MouseMoveEvent& e)
            {
                if (*useDynamicCursor) app->cursor()->setIcon(Cursor::Working);
                else app->cursor()->setIcon(Cursor::Arrow);
            };
            ui_toggleButton->f_onStateChange = [=]
            (ToggleButton::StatefulObject* obj, ToggleButton::StatefulObject::Event& e)
            {
                if (!wk_label1.expired() && !wk_label2.expired())
                {
                    auto sh_label1 = wk_label1.lock();
                    auto sh_label2 = wk_label2.lock();

                    if (e.activated())
                    {
                        app->increaseAnimationCount();

                        *useDynamicCursor = true;
                        app->cursor()->setIcon(Cursor::Working);

                        sh_label1->setText(L"FPS: 0");
                        sh_label2->setText(L"Current cursor: Working");
                    }
                    else if (e.deactivated())
                    {
                        app->decreaseAnimationCount();

                        *useDynamicCursor = false;
                        app->cursor()->setIcon(Cursor::Arrow);

                        sh_label1->setText(L"Animation Mode: Off");
                        sh_label2->setText(L"Current cursor: Arrow");
                    }
                }
            };
            ui_toggleButton->f_onRendererUpdateObject2D = [=](Panel* p, Renderer* rndr)
            {
                if (!wk_label1.expired() && *useDynamicCursor)
                {
                    auto sh_label1 = wk_label1.lock();
                    sh_label1->setText(L"FPS: " + std::to_wstring(rndr->timer()->fps()));
                }
            };
        }
        auto ui_flatButton = makeUIObject<FlatButton>(L"Add X-radius");
        auto ui_filledButton = makeUIObject<FilledButton>(L"Reduce X-radius");
        auto ui_xRadLabel = makeManagedUIObject<Label>(ui_centerLayout);
        {
            ui_flatButton->resize(200.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 0, 1 };
            geoInfo1.axis.y = { 1, 1 };
            ui_centerLayout->addElement(ui_flatButton, geoInfo1);

            ui_filledButton->resize(200.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = true;
            geoInfo2.axis.x = { 0, 1 };
            geoInfo2.axis.y = { 2, 1 };
            ui_centerLayout->addElement(ui_filledButton, geoInfo2);

            ui_xRadLabel->transform(math_utils::offset(ui_flatButton->relativeRect(), { 0.0f, 85.0f }));
            ui_xRadLabel->setText(L"Round radius X: 0px\n(Try wheel above this)");
            THROW_IF_FAILED(ui_xRadLabel->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        }
        auto ui_outlinedButton = makeUIObject<OutlinedButton>(L"Add Y-radius");
        auto ui_elevatedButton = makeUIObject<ElevatedButton>(L"Reduce Y-radius");
        auto ui_yRadLabel = makeManagedUIObject<Label>(ui_centerLayout);
        {
            ui_outlinedButton->resize(200.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo1 = {};
            geoInfo1.isFixedSize = true;
            geoInfo1.axis.x = { 1, 1 };
            geoInfo1.axis.y = { 1, 1 };
            ui_centerLayout->addElement(ui_outlinedButton, geoInfo1);

            ui_elevatedButton->resize(200.0f, 50.0f);

            GridLayout::GeometryInfo geoInfo2 = {};
            geoInfo2.isFixedSize = true;
            geoInfo2.axis.x = { 1, 1 };
            geoInfo2.axis.y = { 2, 1 };
            ui_centerLayout->addElement(ui_elevatedButton, geoInfo2);

            ui_yRadLabel->transform(math_utils::offset(ui_outlinedButton->relativeRect(), { 0.0f, 85.0f }));
            ui_yRadLabel->setText(L"Round radius Y: 0px\n(Try wheel above this)");
            THROW_IF_FAILED(ui_yRadLabel->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
        }
        // Set add/reduce round-radius callbacks.
        {
            std::array<WeakPtr<Button>, 6> wk_buttons =
            {
                ui_button, ui_toggleButton,
                ui_flatButton, ui_filledButton,
                ui_outlinedButton, ui_elevatedButton
            };
            auto wk_labelX = (WeakPtr<Label>)ui_xRadLabel;
            auto wk_labelY = (WeakPtr<Label>)ui_yRadLabel;

            ui_flatButton->f_onMouseButtonRelease = [=]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                int roundRadiusX = 0;
                for (auto& wk_button : wk_buttons)
                {
                    if (!wk_button.expired())
                    {
                        auto sh_button = wk_button.lock();
                        roundRadiusX = (int)++sh_button->roundRadiusX;
                    }
                }
                if (!wk_labelX.expired())
                {
                    wk_labelX.lock()->setText(
                        L"Round radius X: " + std::to_wstring(roundRadiusX) +
                        L"px" + L"\n(Try wheel above this)");
                }
            };
            ui_filledButton->f_onMouseButtonRelease = [=]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                int roundRadiusX = 0;
                for (auto& wk_button : wk_buttons)
                {
                    if (!wk_button.expired())
                    {
                        auto sh_button = wk_button.lock();
                        roundRadiusX = (int)--sh_button->roundRadiusX;
                    }
                }
                if (!wk_labelX.expired())
                {
                    wk_labelX.lock()->setText(
                        L"Round radius X: " + std::to_wstring(roundRadiusX) +
                        L"px" + L"\n(Try wheel above this)");
                }
            };
            ui_outlinedButton->f_onMouseButtonRelease = [=]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                int roundRadiusY = 0;
                for (auto& wk_button : wk_buttons)
                {
                    if (!wk_button.expired())
                    {
                        auto sh_button = wk_button.lock();
                        roundRadiusY = (int)++sh_button->roundRadiusY;
                    }
                }
                if (!wk_labelY.expired())
                {
                    wk_labelY.lock()->setText(
                        L"Round radius Y: " + std::to_wstring(roundRadiusY) +
                        L"px" + L"\n(Try wheel above this)");
                }
            };
            ui_elevatedButton->f_onMouseButtonRelease = [=]
            (ClickablePanel* p, ClickablePanel::Event& e)
            {
                int roundRadiusY = 0;
                for (auto& wk_button : wk_buttons)
                {
                    if (!wk_button.expired())
                    {
                        auto sh_button = wk_button.lock();
                        roundRadiusY = (int)--sh_button->roundRadiusY;
                    }
                }
                if (!wk_labelY.expired())
                {
                    wk_labelY.lock()->setText(
                        L"Round radius Y: " + std::to_wstring(roundRadiusY) +
                        L"px" + L"\n(Try wheel above this)");
                }
            };
            ui_xRadLabel->f_onMouseWheel = [=](Panel* p, MouseWheelEvent& e)
            {
                int roundRadiusX = 0;
                for (auto& wk_button : wk_buttons)
                {
                    if (!wk_button.expired())
                    {
                        auto sh_button = wk_button.lock();
                        roundRadiusX = (int)(sh_button->roundRadiusX += e.deltaCount);
                    }
                }
                ((Label*)p)->setText(
                    L"Round radius X: " + std::to_wstring(roundRadiusX) +
                    L"px" + L"\n(Try wheel above this)");
            };
            ui_yRadLabel->f_onMouseWheel = [=](Panel* p, MouseWheelEvent& e)
            {
                int roundRadiusY = 0;
                for (auto& wk_button : wk_buttons)
                {
                    if (!wk_button.expired())
                    {
                        auto sh_button = wk_button.lock();
                        roundRadiusY = (int)(sh_button->roundRadiusY += e.deltaCount);
                    }
                }
                ((Label*)p)->setText(
                    L"Round radius Y: " + std::to_wstring(roundRadiusY) +
                    L"px" + L"\n(Try wheel above this)");
            };
        }
    });
}
