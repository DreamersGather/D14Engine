#include "Common/Precompile.h"

#include "UIKit/Appearances/ElevatedButton.h"

namespace d14engine::uikit::appearance
{
    void ElevatedButton::Appearance::initialize()
    {
        auto& light = (g_themeData[L"Light"] = {});
        {
            light.shadow.color[(size_t)ButtonState::Idle] =
            light.shadow.color[(size_t)ButtonState::Hover] =
            D2D1::ColorF
            {
                0x808080, // rgb
                1.0f // alpha
            };
            light.shadow.color[(size_t)ButtonState::Down] =
            light.shadow.color[(size_t)ButtonState::Disabled] =
            D2D1::ColorF
            {
                0x000000, // rgb
                0.0f // alpha
            };
        }
        auto& dark = (g_themeData[L"Dark"] = {});
        {
            dark.shadow.color[(size_t)ButtonState::Idle] =
            dark.shadow.color[(size_t)ButtonState::Hover] =
            D2D1::ColorF
            {
                0x000000, // rgb
                1.0f // alpha
            };
            dark.shadow.color[(size_t)ButtonState::Down] =
            dark.shadow.color[(size_t)ButtonState::Disabled] =
            D2D1::ColorF
            {
                0x000000, // rgb
                0.0f // alpha
            };
        }
    }

    void ElevatedButton::Appearance::changeTheme(WstrRefer themeName)
    {
        _D14_FIND_THEME_DATA(themeName);

        _D14_UPDATE_THEME_DATA_ARRAY_1(shadow.color);
    }
    _D14_SET_THEME_DATA_MAP_IMPL(ElevatedButton)
}
