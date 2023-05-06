#include "Common/Precompile.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/OnOffSwitch.h"
#include "UIKit/OutlinedButton.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine;
using namespace d14engine::uikit;

D14_SET_APP_ENTRY(mainDemoTemplate)
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
        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - DemoTemplate @ UIKit");
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
                bitmap_utils::saveBitmap(image.Get(), L"Screenshots/DemoTemplate.png");
            };
        }
    });
}
