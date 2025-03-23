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

#define D14_DEMO_NAME L"DemoTemplate"

#define D14_MAINWINDOW_TITLE L"D14Engine - " D14_DEMO_NAME L" @ UIKit"
#define D14_SCREENSHOT_PATH L"Screenshots/" D14_DEMO_NAME L".png"

D14_SET_APP_ENTRY(mainDemoTemplate)
{
    Application::CreateInfo info =
    {
        .windowSize = { 800, 600 }
    };
    return Application(info).run([](Application* app)
    {
        auto ui_mainWindow = makeRootUIObject<MainWindow>(D14_MAINWINDOW_TITLE);
        {
            ui_mainWindow->bringToFront();
            ui_mainWindow->maximizeButtonEnabled = false;

            ui_mainWindow->caption()->transform(300.0f, 0.0f, 376.0f, 32.0f);
        }
        auto ui_darkModeLabel = makeRootUIObject<Label>(L"Dark Mode");
        auto ui_darkModeSwitch = makeRootUIObject<OnOffSwitch>();
        {
            ui_darkModeLabel->bringToFront();
            ui_darkModeLabel->transform(10.0f, 0.0f, 120.0f, 32.0f);

            ui_darkModeSwitch->bringToFront();
            ui_darkModeSwitch->setPosition(130.0f, 4.0f);

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
            ui_screenshot->bringToFront();
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
        ////////////////////////////////////////
        //========== insert code here ==========
        ////////////////////////////////////////
    });
}
