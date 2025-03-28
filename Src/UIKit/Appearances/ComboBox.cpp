﻿#include "Common/Precompile.h"

#include "UIKit/Appearances/ComboBox.h"

namespace d14engine::uikit::appearance
{
    void ComboBox::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"]);
        {
            light.main[(size_t)ButtonState::Idle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xfdfdfd }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                }
            };
            light.main[(size_t)ButtonState::Hover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xf9f9f9 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                }
            };
            light.main[(size_t)ButtonState::Down] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.65f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xf5f5f5 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                }
            };
            light.main[(size_t)ButtonState::Disabled] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x9e9e9e }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xf3f3f3 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0xbfbfbf }, // color
                    1.0f // opacity
                }
            };
            light.arrow.background.color = D2D1::ColorF{ 0x000000 };
            light.arrow.secondaryBackground.color = D2D1::ColorF{ 0x9e9e9e };
        }
        auto& dark = (g_themeData[L"Dark"]);
        {
            dark.main[(size_t)ButtonState::Idle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x343434 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0x1d1d1d }, // color
                    1.0f // opacity
                }
            };
            dark.main[(size_t)ButtonState::Hover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x393939 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0x1d1d1d }, // color
                    1.0f // opacity
                }
            };
            dark.main[(size_t)ButtonState::Down] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    0.55f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x2e2e2e }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0x1d1d1d }, // color
                    1.0f // opacity
                }
            };
            dark.main[(size_t)ButtonState::Disabled] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x777777 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x202020 }, // color
                    1.0f // opacity
                },
                // stroke
                {
                    1.0f, // width
                    D2D1::ColorF{ 0x434343 }, // color
                    1.0f // opacity
                }
            };
            dark.arrow.background.color = D2D1::ColorF{ 0xffffff };
            dark.arrow.secondaryBackground.color = D2D1::ColorF{ 0x777777 };
        }
    }

    void ComboBox::Appearance::changeTheme(Master& m, WstrRefer themeName)
    {
        _D14_FIND_THEME_DATA(themeName);

        _D14_UPDATE_THEME_DATA_ARRAY_2(main);

        _D14_UPDATE_THEME_DATA_1(arrow.background.color);
        _D14_UPDATE_THEME_DATA_1(arrow.secondaryBackground.color);
    }
    _D14_SET_THEME_DATA_MAP_IMPL(ComboBox)
}
