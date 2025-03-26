#include "Common/Precompile.h"

#include "UIKit/Appearances/ToggleButton.h"

namespace d14engine::uikit::appearance
{
    void ToggleButton::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"] = {});
        {
            light.main[(size_t)ButtonState::Idle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.main[(size_t)ButtonState::Hover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.main[(size_t)ButtonState::Down] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    0.65f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.main[(size_t)ButtonState::Disabled] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xbfbfbf }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
        }
        auto& dark = (g_themeData[L"Dark"] = {});
        {
            dark.main[(size_t)ButtonState::Idle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.main[(size_t)ButtonState::Hover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.main[(size_t)ButtonState::Down] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.55f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.main[(size_t)ButtonState::Disabled] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xa8a8a8 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x434343 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    0.0f, // width
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
        }
    }

    void ToggleButton::Appearance::changeTheme(WstrRefer themeName)
    {
        _D14_FIND_THEME_DATA(themeName);

#define UPDATE_BUTTON_COLOR(Btn_State, Btn_Color) \
do { \
    constexpr auto index = (size_t)ButtonState::Btn_State; \
    _ref.main[index].background.color = Btn_Color(); \
} while (0)
        UPDATE_BUTTON_COLOR(Idle, color1);
        UPDATE_BUTTON_COLOR(Hover, color2);
        UPDATE_BUTTON_COLOR(Down, color3);

#undef UPDATE_COLOR
        _D14_UPDATE_THEME_DATA_ARRAY_1(main);
    }
    _D14_SET_THEME_DATA_MAP_IMPL(ToggleButton)
}
