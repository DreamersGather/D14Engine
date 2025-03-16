#include "Common/Precompile.h"

#include "UIKit/Appearances/Window.h"

namespace d14engine::uikit::appearance
{
    Window::Appearance::Appearance()
    {
        stroke.opacity = 0.0f;
    }

    void Window::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"] = {});
        {
            light.background.color = D2D1::ColorF{ 0xf9f9f9 };

            light.shadow.color = D2D1::ColorF{ 0x808080 };

            light.captionPanel.background =
            {
                D2D1::ColorF{ 0xf3f3f3 } // color
            };
            light.buttonPanel[(size_t)ButtonState::Idle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.8f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.buttonPanel[(size_t)ButtonState::Hover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.8f // opacity
                }
            };
            light.buttonPanel[(size_t)ButtonState::Down] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    0.65f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.65f // opacity
                }
            };
            light.buttonPanel[(size_t)ButtonState::CloseIdle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            light.buttonPanel[(size_t)ButtonState::CloseHover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xe81123 }, // color
                    0.8f // opacity
                }
            };
            light.buttonPanel[(size_t)ButtonState::CloseDown] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.65f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xf1707a }, // color
                    0.65f // opacity
                }
            };
        }
        auto& dark = (g_themeData[L"Dark"] = {});
        {
            dark.background.color = D2D1::ColorF{ 0x272727 };

            dark.shadow.color = D2D1::ColorF{ 0x000000 };

            dark.captionPanel.background =
            {
                D2D1::ColorF{ 0x202020 } // color
            };
            dark.buttonPanel[(size_t)ButtonState::Idle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    0.8f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.buttonPanel[(size_t)ButtonState::Hover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xe5e5e5 }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x666666 }, // color
                    0.8f // opacity
                }
            };
            dark.buttonPanel[(size_t)ButtonState::Down] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    0.55f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x666666 }, // color
                    0.55f // opacity
                }
            };
            dark.buttonPanel[(size_t)ButtonState::CloseIdle] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.0f // opacity
                }
            };
            dark.buttonPanel[(size_t)ButtonState::CloseHover] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0xffffff }, // color
                    1.0f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xe81123 }, // color
                    0.8f // opacity
                }
            };
            dark.buttonPanel[(size_t)ButtonState::CloseDown] =
            {
                // foreground
                {
                    D2D1::ColorF{ 0x000000 }, // color
                    0.55f // opacity
                },
                // background
                {
                    D2D1::ColorF{ 0xf1707a }, // color
                    0.55f // opacity
                }
            };
        }
    }

    void Window::Appearance::changeTheme(WstrRefer themeName)
    {
        stroke.color = color::normal();

        decorativeBar.gradientColors[0] = color3();
        decorativeBar.gradientColors[1] = color2();
        decorativeBar.gradientColors[2] = color1();

        _D14_FIND_THEME_DATA(themeName);

        _D14_UPDATE_THEME_DATA_1(background.color);
        _D14_UPDATE_THEME_DATA_1(shadow.color);

        _D14_UPDATE_THEME_DATA_1(captionPanel.background.color);

        _D14_UPDATE_THEME_DATA_ARRAY_1(buttonPanel);
    }
    _D14_SET_THEME_DATA_MAP_IMPL(Window)
}
