#include "Common/Precompile.h"

#include "UIKit/Appearances/OutlinedButton.h"

namespace d14engine::uikit::appearance
{
    void OutlinedButton::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"] = {});
        {
            light.main[(size_t)ButtonState::Disabled] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x9e9e9e }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0xbfbfbf }, // color
                    1.0f // opacity
                }
            };
        }
        auto& dark = (g_themeData[L"Dark"] = {});
        {
            dark.main[(size_t)ButtonState::Disabled] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x777777 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0x434343 }, // color
                    1.0f // opacity
                }
            };
        }
    }

    void OutlinedButton::Appearance::changeTheme(Master& m, WstrRefer themeName)
    {
        _D14_FIND_THEME_DATA(themeName);

        _ref.main[(size_t)ButtonState::Idle] =
        {
            // foreground
            {
                color1(), // color
                1.0f // opacity
            },
            // background
            {
                D2D1::ColorF{ 0x000000 }, // color
                0.0f // opacity
            },
            // stroke
            {
                1.0f, // width
                color1(), // color
                1.0f // opacity
            }
        };
        _ref.main[(size_t)ButtonState::Hover] =
        {
            // foreground
            {
                color2(), // color
                1.0f // opacity
            },
            // background
            {
                color2(), // color
                0.05f // opacity
            },
            // stroke
            {
                1.0f, // width
                color2(), // color
                1.0f // opacity
            }
        };
        _ref.main[(size_t)ButtonState::Down] =
        {
            // foreground
            {
                color3(), // color
                0.65f // opacity
            },
            // background
            {
                color3(), // color
                0.12f // opacity
            },
            // stroke
            {
                1.0f, // width
                color3(), // color
                1.0f // opacity
            }
        };
        _D14_UPDATE_THEME_DATA_ARRAY_2(main);
    }
    _D14_SET_THEME_DATA_MAP_IMPL(OutlinedButton)
}
