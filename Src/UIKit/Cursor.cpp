#include "Common/Precompile.h"

#include "UIKit/Cursor.h"

#include "Common/MathUtils/2D.h"
#include "Common/RuntimeError.h"

#include "UIKit/Application.h"
#include "UIKit/BitmapUtils.h"
#include "UIKit/FileSystemUtils.h"

using namespace d14engine::renderer;
using namespace fanim_literals;

namespace d14engine::uikit
{
    Cursor::Cursor(const BasicIconThemeMap& icons, const D2D1_RECT_F& rect)
        :
        Panel(rect),
        m_classifiedBasicIcons(icons)
    {
        // Keep the cursor always displayed at the top.
        ISortable<IDrawObject2D>::m_priority = INT_MAX;
    }

    void Cursor::registerDrawObjects()
    {
        THROW_IF_NULL(Application::g_app);

        auto& uiCmdLayer = Application::g_app->uiCmdLayer();
        if (std::holds_alternative<Renderer::CommandLayer::D2D1Target>(uiCmdLayer->drawTarget))
        {
            auto& drawobjs2d = std::get<Renderer::CommandLayer::D2D1Target>(uiCmdLayer->drawTarget);
            drawobjs2d.insert(shared_from_this());
        }
        // No need to update the topmost draw-object priority
        // since the cursor should always be displayed at the top.
    }

    Cursor::BasicIconThemeMap Cursor::loadBasicIcons()
    {
        return
        {
            { L"Light", loadBasicIconSeries(L"Light") },
            { L"Dark", loadBasicIconSeries(L"Dark") }
        };
    }

    Cursor::IconSeries Cursor::loadBasicIconSeries(WstrRefer themeName)
    {
        THROW_IF_NULL(Application::g_app);

        IconSeries icons = {};

        auto cursorPath = Application::g_app->createInfo.cursorPath() + themeName + L"/";

        ////////////////////////
        // Load static icons. //
        ////////////////////////

#define DEF_STATIC_ICON(Name, ...) { StaticIconIndex::Name, L#Name L".png ", __VA_ARGS__ }

        std::tuple<StaticIconIndex, Wstring, D2D1_POINT_2F> staticIconPaths[] =
        {
            DEF_STATIC_ICON(Alternate, { 30.0f, 0.0f }),
            DEF_STATIC_ICON(Arrow,     { 2.0f, 0.0f }),
            DEF_STATIC_ICON(BackDiag,  { 16.0f, 16.0f }),
            DEF_STATIC_ICON(Hand,      { 3.0f, 3.0f }),
            DEF_STATIC_ICON(Help,      { 3.0f, 3.0f }),
            DEF_STATIC_ICON(HorzSize,  { 16.0f, 16.0f }),
            DEF_STATIC_ICON(MainDiag,  { 16.0f, 16.0f }),
            DEF_STATIC_ICON(Move,      { 16.0f, 16.0f }),
            DEF_STATIC_ICON(Person,    { 2.0f, 0.0f }),
            DEF_STATIC_ICON(Pin,       { 2.0f, 0.0f }),
            DEF_STATIC_ICON(Select,    { 16.0f, 16.0f }),
            DEF_STATIC_ICON(Stop,      { 2.0f, 0.0f }),
            DEF_STATIC_ICON(Text,      { 16.0f, 16.0f }),
            DEF_STATIC_ICON(VertSize,  { 16.0f, 16.0f })
        };

#undef DEF_STATIC_ICON

        for (auto& path : staticIconPaths)
        {
            icons.staticIcons[(size_t)std::get<0>(path)] =
            {
                bitmap_utils::loadBitmap(cursorPath + std::get<1>(path)),
                std::get<2>(path) // hot spot offset
            };
        }

        /////////////////////////
        // Load dynamic icons. //
        /////////////////////////

#define LOAD_DYNAMIC_ICON(Name, ...) \
do { \
    auto frames = loadBasicIconFrames(cursorPath + L#Name L"/"); \
    frames.hotSpotOffset = __VA_ARGS__; \
    icons.dynamicIcons[(size_t)DynamicIconIndex::Name] = std::move(frames); \
} while (0)

        LOAD_DYNAMIC_ICON(Busy,    { 16.0f, 16.0f });
        LOAD_DYNAMIC_ICON(Working, { 2.0f, 0.0f });

#undef LOAD_DYNAMIC_ICON

        return icons;
    }

    Cursor::DynamicIcon Cursor::loadBasicIconFrames(WstrRefer framesPath)
    {
        DynamicIcon icon = {};

        /////////////////
        // Load Images //
        /////////////////

        animation_utils::BitmapSequence::FramePackage frames = {};

        file_system_utils::foreachFileInDir
        (framesPath, L"*.png", [&](WstrRefer path)
        {
            auto name = file_system_utils::extractFilePrefix(
                        file_system_utils::extractFileName(path));

            frames[name] = bitmap_utils::loadBitmap(path);

            return false;
        });

        //////////////////
        // Setup Frames //
        //////////////////

        auto& fanim = icon.bitmapData.fanim;

        fanim.frames.resize(frames.size());
        for (auto& f : frames)
        {
            auto index = (size_t)(std::stoi(f.first) - 1);
            if (index >= 0_uz && index < fanim.frames.size())
            {
                fanim.frames[index] = f.second;
            }
        }
        fanim.timeSpanDataInSecs = 2_jf;

        return icon;
    }

    void Cursor::registerIcon(WstrRefer themeName, StaticIconIndex index, const StaticIcon& icon)
    {
        auto categoryItor = m_classifiedBasicIcons.find(themeName);
        if (categoryItor != m_classifiedBasicIcons.end())
        {
            categoryItor->second.staticIcons[(size_t)index] = icon;
        }
        else (m_classifiedBasicIcons[themeName] = {}).staticIcons[(size_t)index] = icon;
    }

    void Cursor::registerIcon(WstrRefer name, const StaticIcon& icon)
    {
        m_customIcons.staticIcons[name] = icon;
    }

    void Cursor::unregisterStaticIcon(WstrRefer name)
    {
        m_customIcons.staticIcons.erase(name);
    }

    void Cursor::registerIcon(WstrRefer themeName, DynamicIconIndex index, const DynamicIcon& icon)
    {
        auto categoryItor = m_classifiedBasicIcons.find(themeName);
        if (categoryItor != m_classifiedBasicIcons.end())
        {
            categoryItor->second.dynamicIcons[(size_t)index] = icon;
        }
        else (m_classifiedBasicIcons[themeName] = {}).dynamicIcons[(size_t)index] = icon;
    }

    void Cursor::registerIcon(WstrRefer name, const DynamicIcon& icon)
    {
        m_customIcons.dynamicIcons[name] = icon;
    }

    void Cursor::unregisterDynamicIcon(WstrRefer name)
    {
        m_customIcons.dynamicIcons.erase(name);
    }

    void Cursor::setIcon(StaticIconIndex index)
    {
        THROW_IF_NULL(Application::g_app);

        m_selectedIconID = index;

        if (m_iconSource == System && !m_systemIconUpdateFlag)
        {
            m_systemIconUpdateFlag = true;
            PostMessage(Application::g_app->win32Window(), WM_SETCURSOR, 0, HTCLIENT);
        }
    }

    void Cursor::setStaticIcon(WstrRefer name)
    {
        m_selectedIconID.emplace<g_staticIconSeat>(name);
    }

    void Cursor::setIcon(DynamicIconIndex index)
    {
        THROW_IF_NULL(Application::g_app);

        m_selectedIconID = index;

        if (m_iconSource == System && !m_systemIconUpdateFlag)
        {
            m_systemIconUpdateFlag = true;
            PostMessage(Application::g_app->win32Window(), WM_SETCURSOR, 0, HTCLIENT);
        }
    }

    void Cursor::setDynamicIcon(WstrRefer name)
    {
        m_selectedIconID.emplace<g_dynamicIconSeat>(name);
    }

    Cursor::IconSource Cursor::iconSource() const
    {
        return m_iconSource;
    }

    void Cursor::setIconSource(IconSource src)
    {
        if ((m_iconSource = src) == System)
        {
            PostMessage(Application::g_app->win32Window(), WM_SETCURSOR, 0, HTCLIENT);
        }
    }

    void Cursor::setSystemIcon()
    {
        if (m_visible)
        {
#define SET_CURSOR(Icon_Name) SetCursor(LoadCursor(nullptr, Icon_Name)); break

            if (m_selectedIconID.index() == g_staticIconSeat)
            {
                auto& iconID0 = std::get<g_staticIconSeat>(m_selectedIconID);
                if (iconID0.index() == g_basicIconSeat)
                {
                    auto& iconID = std::get<g_basicIconSeat>(iconID0);
                    switch (iconID)
                    {
                    case Alternate: SET_CURSOR(IDC_UPARROW);
                    case Arrow:     SET_CURSOR(IDC_ARROW);
                    case BackDiag:  SET_CURSOR(IDC_SIZENESW);
                    case Hand:      SET_CURSOR(IDC_HAND);
                    case Help:      SET_CURSOR(IDC_HELP);
                    case HorzSize:  SET_CURSOR(IDC_SIZEWE);
                    case MainDiag:  SET_CURSOR(IDC_SIZENWSE);
                    case Move:      SET_CURSOR(IDC_SIZEALL);
                    case Person:    SET_CURSOR(IDC_PERSON);
                    case Pin:       SET_CURSOR(IDC_PIN);
                    case Select:    SET_CURSOR(IDC_CROSS);
                    case Stop:      SET_CURSOR(IDC_NO);
                    case Text:      SET_CURSOR(IDC_IBEAM);
                    case VertSize:  SET_CURSOR(IDC_SIZENS);
                    default: SetCursor(nullptr); break;
                    }
                    return;
                }
            }
            else if (m_selectedIconID.index() == g_dynamicIconSeat)
            {
                auto& iconID0 = std::get<g_dynamicIconSeat>(m_selectedIconID);
                if (iconID0.index() == g_basicIconSeat)
                {
                    auto& iconID = std::get<g_basicIconSeat>(iconID0);
                    switch (iconID)
                    {
                    case Busy:    SET_CURSOR(IDC_WAIT);
                    case Working: SET_CURSOR(IDC_APPSTARTING);
                    default: SetCursor(nullptr); break;
                    }
                    return;
                }
            }
#undef SET_CURSOR
        }
        SetCursor(nullptr);
    }

    Cursor::StaticIcon& Cursor::getCurrentSelectedStaticIcon()
    {
        THROW_IF_NULL(Application::g_app);

        auto& iconID = std::get<g_staticIconSeat>(m_selectedIconID);
        auto& basicIcons = m_classifiedBasicIcons.at(Application::g_app->themeStyle().name);

        return (iconID.index() == g_basicIconSeat) ?
            basicIcons.staticIcons[(size_t)std::get<g_basicIconSeat>(iconID)] :
            m_customIcons.staticIcons[std::get<g_customIconSeat>(iconID)];
    }

    Cursor::DynamicIcon& Cursor::getCurrentSelectedDynamicIcon()
    {
        THROW_IF_NULL(Application::g_app);

        auto& iconID = std::get<g_dynamicIconSeat>(m_selectedIconID);
        auto& basicIcons = m_classifiedBasicIcons.at(Application::g_app->themeStyle().name);

        return (iconID.index() == g_basicIconSeat) ?
            basicIcons.dynamicIcons[(size_t)std::get<g_basicIconSeat>(iconID)] :
            m_customIcons.dynamicIcons[std::get<g_customIconSeat>(iconID)];
    }

    void Cursor::onRendererUpdateObject2DHelper(Renderer* rndr)
    {
        if (m_iconSource == UIKit)
        {
            if (m_selectedIconID.index() == g_dynamicIconSeat)
            {
                getCurrentSelectedDynamicIcon().bitmapData.update(rndr);
            }
        }
    }

    void Cursor::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        if (m_iconSource == UIKit)
        {
            if (m_lastSelectedIconID.index() == g_staticIconSeat &&
                m_selectedIconID.index() == g_dynamicIconSeat)
            {
                getCurrentSelectedDynamicIcon().bitmapData.restore();
                increaseAnimationCount();
            }
            if (m_lastSelectedIconID.index() == g_dynamicIconSeat &&
                m_selectedIconID.index() == g_staticIconSeat)
            {
                decreaseAnimationCount();
            }
            m_lastSelectedIconID = m_selectedIconID;

            if (m_selectedIconID.index() == g_staticIconSeat)
            {
                auto& icon = getCurrentSelectedStaticIcon();

                auto hs = math_utils::minus(icon.hotSpotOffset);
                auto rect = math_utils::offset(m_absoluteRect, hs);

                auto& bmpobj = icon.bitmapData;
                rndr->d2d1DeviceContext()->DrawBitmap
                (
                /* bitmap               */ bmpobj.data.Get(),
                /* destinationRectangle */ rect,
                /* opacity              */ bmpobj.opacity,
                /* interpolationMode    */ bmpobj.getInterpolationMode()
                );
            }
            else if (m_selectedIconID.index() == g_dynamicIconSeat)
            {
                auto& icon = getCurrentSelectedDynamicIcon();

                auto hs = math_utils::minus(icon.hotSpotOffset);
                auto rect = math_utils::offset(m_absoluteRect, hs);

                icon.bitmapData.draw(rndr, rect);
            }
        }
    }
}
