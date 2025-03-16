#include "Common/Precompile.h"

#include "UIKit/Appearances/ScrollView.h"

namespace d14engine::uikit::appearance
{
    ScrollView::Appearance::Appearance()
    {
        //--------------------------------------------------------------
        // 1. Grayscale text anti-aliasing:
        // The rendering result is independent of the target background,
        // so opacity can be set as needed (any value from 0 ~ 1 is OK).
        //--------------------------------------------------------------
        // 2. ClearType text anti-aliasing:
        // The rendering result depends on the target background color,
        // so you must set an opaque background (better a value >= 0.5).
        //--------------------------------------------------------------
        background.opacity = 0.0f;
        stroke.opacity = 0.0f;

        scrollBar[(size_t)ScrollBarState::Idle].geometry =
        {
            4.0f, // width
            2.0f, // round radius
            2.0f  // offset
        };
        scrollBar[(size_t)ScrollBarState::Hover].geometry =
        scrollBar[(size_t)ScrollBarState::Down].geometry =
        {
            8.0f, // width
            4.0f, // round radius
            2.0f  // offset
        };
    }

    void ScrollView::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"] = {});
        {
            light.background.color = D2D1::ColorF{ 0xf3f3f3 };
            light.stroke.color = D2D1::ColorF{ 0xe5e5e5 };

            light.scrollBar[(size_t)ScrollBarState::Idle].background =
            light.scrollBar[(size_t)ScrollBarState::Hover].background =
            {
                D2D1::ColorF{ 0xc2c2c2 }, // color
                0.8f // opacity
            };
            light.scrollBar[(size_t)ScrollBarState::Down].background =
            {
                D2D1::ColorF{ 0xa8a8a8 }, // color
                0.8f // opacity
            };
        }
        auto& dark = (g_themeData[L"Dark"] = {});
        {
            dark.background.color = D2D1::ColorF{ 0x202020 };
            dark.stroke.color = D2D1::ColorF{ 0x1d1d1d };

            dark.scrollBar[(size_t)ScrollBarState::Idle].background =
            dark.scrollBar[(size_t)ScrollBarState::Hover].background =
            {
                D2D1::ColorF{ 0x757575 }, // color
                0.8f // opacity
            };
            dark.scrollBar[(size_t)ScrollBarState::Down].background =
            {
                D2D1::ColorF{ 0x8f8f8f }, // color
                0.8f // opacity
            };
        }
    }

    void ScrollView::Appearance::changeTheme(WstrRefer themeName)
    {
        _D14_FIND_THEME_DATA(themeName);

        _D14_UPDATE_THEME_DATA_1(background.color);
        _D14_UPDATE_THEME_DATA_1(stroke.color);

        for (size_t i = 0; i < g_stateCount; ++i)
        {
            scrollBar[i].background = _ref.scrollBar[i].background;
        }
    }
    _D14_SET_THEME_DATA_MAP_IMPL(ScrollView)
}
