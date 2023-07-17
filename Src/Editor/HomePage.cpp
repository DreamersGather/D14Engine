#include "Common/Precompile.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/MainWindow.h"

using namespace d14engine;
using namespace d14engine::uikit;

D14_SET_APP_ENTRY(mainEditor)
{
    Application::CreateInfo info = {};
    if (argc >= 2 && lstrcmp(argv[1], L"HighDPI") == 0)
    {
        info.dpi = 192.0f;
    }
    else info.dpi = 96.0f;

    return Application(argc, argv, info).run([&](Application* app)
    {
        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - Editor");
        {
            ui_mainWindow->setCaptionPanelHeight(40.0f);
            ui_mainWindow->setDecorativeBarHeight(2.0f);

            ui_mainWindow->setDisplayState(Window::DisplayState::Maximized);
        }
    });
}
