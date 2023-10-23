#include "Common/Precompile.h"

#include "Common/MathUtils/2D.h"

#include "Renderer/Renderer.h"
#include "Renderer/TickTimer.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/PlatformUtils.h"
#include "UIKit/ScenePanel.h"

using namespace d14engine;
using namespace d14engine::renderer;
using namespace d14engine::uikit;

#include "TestWindow.h"

D14_SET_APP_ENTRY(mainTestWindow)
{
    Application::CreateInfo info = {};
    if (argc >= 2 && lstrcmp(argv[1], L"HighDPI") == 0)
    {
        info.dpi = 192.0f;
    }
    else info.dpi = 96.0f;
    info.win32WindowRect = { 0, 0, 1280, 756 };

    BitmapObject::g_interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;

    return Application(argc, argv, info).run([&](Application* app)
    {
        app->dxRenderer()->dxgiFactoryInfo().setting.setAllowTearing(true);

        app->win32WindowSettings.geometry.minTrackSize = { 1280, 756 };
        app->win32WindowSettings.sizingFrame.frameWidth = 6;

        app->increaseAnimationCount(); // enable renderer updating

        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - TestWindow @ Pipeline");

        auto ui_scenePanel = makeUIObject<ScenePanel>();
        {
            ui_mainWindow->setCenterUIObject(ui_scenePanel);

            ui_scenePanel->f_onChangeTheme = [](Panel* p, WstrParam themeName)
            {
                auto sp = (ScenePanel*)p;
                if (themeName == L"Light") sp->setClearColor(Colors::White);
                else if (themeName == L"Dark") sp->setClearColor(Colors::Black);
            };
            ui_scenePanel->f_onChangeTheme(ui_scenePanel.get(), app->currThemeName());
        }
        auto ui_fpsLabel = makeManagedUIObject<Label>(ui_scenePanel, L"FPS: ");
        {
            ui_fpsLabel->move(10.0f, 10.0f);
            ui_fpsLabel->hardAlignment.vert = Label::VertAlignment::Top;

            ui_fpsLabel->f_onRendererUpdateObject2DAfter = [](Panel* p, Renderer* rndr)
            {
                static UINT fps = 0;
                if (rndr->timer()->fps() != fps)
                {
                    fps = rndr->timer()->fps();
                    ((Label*)p)->setText(L"FPS: " + std::to_wstring(fps));
                }
            };
        }
        auto pixSize = platform_utils::scaledByDpi(SIZE{ 1280, 720 });
        auto strw = std::to_wstring((int)pixSize.cx);
        auto strh = std::to_wstring((int)pixSize.cy);
        auto resText = L"Resolution: " + strw + L" x " + strh;
        auto ui_resLabel = makeManagedUIObject<Label>(ui_scenePanel, resText);
        {
            ui_resLabel->move(10.0f, 20.0f + ui_fpsLabel->textAreaSize().height);
            ui_resLabel->hardAlignment.vert = Label::VertAlignment::Top;

            auto wk_resLabel = (WeakPtr<Label>)ui_resLabel;
            ui_scenePanel->f_onSize = [wk_resLabel](Panel* p, SizeEvent& e)
            {
                if (!wk_resLabel.expired())
                {
                    auto dipSize = math_utils::roundu(e.size);
                    SIZE sz = { (LONG)dipSize.width, (LONG)dipSize.height };
                    auto pixSize = platform_utils::scaledByDpi(sz);
                    auto strw = std::to_wstring((int)pixSize.cx);
                    auto strh = std::to_wstring((int)pixSize.cy);
                    wk_resLabel.lock()->setText(L"Resolution: " + strw + L" x " + strh);
                }
            };
        }
        initTestScene(app, ui_scenePanel.get());
    });
}
