#include "Common/Precompile.h"

#include "UIKit/Appearances/MenuItem.h"

namespace d14engine::uikit::appearance
{
    void MenuItem::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"] = {});
        {
            light.main[(size_t)ViewItem::State::Idle] =
            {
                // background
                {
                    D2D1::ColorF{ 0xf9f9f9 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.main[(size_t)ViewItem::State::Hover] =
            {
                // background
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.main[(size_t)ViewItem::State::FocusSelected] =
            light.main[(size_t)ViewItem::State::Selected] =
            light.main[(size_t)ViewItem::State::FocusSelectedHover] =
            {
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.arrow.background =
            {
                D2D1::ColorF{ 0x000000 }, // color
                1.0f // opacity
            };
        }
        auto& dark = (g_themeData[L"Dark"] = {});
        {
            dark.main[(size_t)ViewItem::State::Idle] =
            {
                // background
                {
                    D2D1::ColorF{ 0x272727 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.main[(size_t)ViewItem::State::Hover] =
            {
                // background
                {
                    D2D1::ColorF{ 0x3d3d3d }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.main[(size_t)ViewItem::State::FocusSelected] =
            dark.main[(size_t)ViewItem::State::Selected] =
            dark.main[(size_t)ViewItem::State::FocusSelectedHover] =
            {
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.arrow.background =
            {
                D2D1::ColorF{ 0xffffff }, // color
                1.0f // opacity
            };
        }
    }

    void MenuItem::Appearance::changeTheme(Master& m, WstrRefer themeName)
    {
        _D14_FIND_THEME_DATA(themeName);

        _D14_UPDATE_THEME_DATA_ARRAY_2(main);
        _D14_UPDATE_THEME_DATA_1(arrow.background);
    }
    _D14_SET_THEME_DATA_MAP_IMPL(MenuItem)
}
