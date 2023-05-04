#include "Common/Precompile.h"

#include "Common/MathUtils/2D.h"
#include "Common/MathUtils/GDI.h"

#include "UIKit/AppEntry.h"
#include "UIKit/Application.h"
#include "UIKit/ConstraintLayout.h"
#include "UIKit/IconLabel.h"
#include "UIKit/Label.h"
#include "UIKit/MainWindow.h"
#include "UIKit/ResourceUtils.h"
#include "UIKit/TabCaption.h"
#include "UIKit/TabGroup.h"

using namespace d14engine;
using namespace d14engine::uikit;

#include "TabPages/Gallery/Gallery.h"
#include "TabPages/Settings/Settings.h"

D14_SET_APP_ENTRY(mainWidgetsGallery)
{
    Application::CreateInfo info = {};
    info.dpi = 96.0f;
    info.win32WindowRect = { 0, 0, 1280, 720 };

    BitmapObject::g_interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;

    return Application(argc, argv, info).run([&](Application* app)
    {
        auto ui_mainWindow = makeRootUIObject<MainWindow>(L"D14Engine - WidgetsGallery @ UIKit");
        {
            ui_mainWindow->setCaptionPanelHeight(40.0f);
            ui_mainWindow->setDecorativeBarHeight(2.0f);

            ui_mainWindow->moveTopmost();
        }
        auto ui_centerLayout = makeUIObject<ConstraintLayout>();
        {
            ui_mainWindow->setCenterUIObject(ui_centerLayout);
        }
        auto ui_tabGroup = makeUIObject<TabGroup>();
        {
            ConstraintLayout::GeometryInfo geoInfo = {};

            geoInfo.keepWidth = false;
            geoInfo.Left.ToLeft = 0.0f; geoInfo.Right.ToRight = 0.0f;

            geoInfo.keepHeight = false;
            geoInfo.Top.ToTop = 50.0f; geoInfo.Bottom.ToBottom = 0.0f;

            ui_centerLayout->addElement(ui_tabGroup, geoInfo);

#define SET_CARD_SIZE(State, Width, Height) \
    ui_tabGroup->getAppearance().tabBar.card.main \
    [(size_t)TabGroup::CardState::State].geometry.size = { Width, Height }

            SET_CARD_SIZE(Dormant, 250.0f,  32.0f);
            SET_CARD_SIZE(Hover,   250.0f,  32.0f);
            SET_CARD_SIZE(Active,  266.0f,  40.0f);

#undef SET_CARD_SIZE
            ui_tabGroup->loadActiveCardShadowBitmap();

            auto& barAppear = ui_tabGroup->getAppearance().tabBar;

            barAppear.geometry.height = 40.0f;
            barAppear.separator.geometry.size.height = 24.0f;
            barAppear.moreCards.control.button.geometry.offset.y = 7.0f;
        }
        auto appendTabPage = [&](WstrParam title)
        {
            auto ui_caption = makeUIObject<TabCaption>(title);
            auto ui_content = makeUIObject<ConstraintLayout>();
            
            ui_caption->title()->label()->setTextFormat(D14_FONT(L"Default/Normal/14"));

            ui_caption->closable = false;
            ui_caption->draggable = false;

            ui_tabGroup->appendTab({ ui_caption, ui_content });

            return ui_content;
        };
        createGalleryTabPage(appendTabPage(L"Gallery").get());
        createSettingsTabPage(appendTabPage(L"Settings").get());

        ui_tabGroup->selectTab(0);

        app->win32WindowSettings.geometry.minTrackSize = { 1280, 720 };

        app->win32WindowSettings.sizingFrame.frameWidth = 6;
        app->win32WindowSettings.sizingFrame.f_NCHITTEST = [=]
        (const POINT& pt) -> LRESULT
        {
            auto frmWidth = app->win32WindowSettings.sizingFrame.frameWidth.value();

            RECT clntRect = {};
            GetClientRect(app->win32Window(), &clntRect);
            auto clntSize = math_utils::size(clntRect);

            // Escape the area of tab group card bar.
            if (pt.y >= 42 && pt.y <= 92)
            {
                return HTCLIENT;
            }
            // Escape the area of main window 3 brothers.
            if (pt.y <= frmWidth && pt.x >= (clntSize.cx - 124) && pt.x <= (clntSize.cx - 20))
            {
                return HTCLIENT;
            }
            // Escape the area of gallery page side category.
            if ((pt.x <= frmWidth && pt.y >= 92) || (pt.x <= 350 && pt.y >= (clntSize.cy - frmWidth)))
            {
                return HTCLIENT;
            }
            return app->defWin32NCHITTESTMessageHandler(pt);
        };
    });
}
