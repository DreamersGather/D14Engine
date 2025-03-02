#include "Common/Precompile.h"

#include "Common/DirectXError.h"

#include "UIKit/Application.h"
#include "UIKit/ColorUtils.h"

using namespace d14engine::uikit::color_utils;

#include "UIKit/Appearances/Button.h"
#include "UIKit/Appearances/CheckBox.h"
#include "UIKit/Appearances/ComboBox.h"
#include "UIKit/Appearances/ElevatedButton.h"
#include "UIKit/Appearances/FilledButton.h"
#include "UIKit/Appearances/FlatButton.h"
#include "UIKit/Appearances/Label.h"
#include "UIKit/Appearances/LabelArea.h"
#include "UIKit/Appearances/Layout.h"
#include "UIKit/Appearances/MenuItem.h"
#include "UIKit/Appearances/MenuSeparator.h"
#include "UIKit/Appearances/OnOffSwitch.h"
#include "UIKit/Appearances/OutlinedButton.h"
#include "UIKit/Appearances/PopupMenu.h"
#include "UIKit/Appearances/RawTextInput.h"
#include "UIKit/Appearances/ResizablePanel.h"
#include "UIKit/Appearances/ScrollView.h"
#include "UIKit/Appearances/Slider.h"
#include "UIKit/Appearances/TabCaption.h"
#include "UIKit/Appearances/TabGroup.h"
#include "UIKit/Appearances/TextInput.h"
#include "UIKit/Appearances/ToggleButton.h"
#include "UIKit/Appearances/TreeViewItem.h"
#include "UIKit/Appearances/ViewItem.h"
#include "UIKit/Appearances/Window.h"

namespace d14engine::uikit::appearance
{
    void initialize()
    {
        THROW_IF_NULL(Application::g_app);

        generateTonedColors(Application::g_app->themeStyle());

#define INIT_THEME_DATA(Class_Name) \
        Class_Name::Appearance::initialize()

        INIT_THEME_DATA(Button);
        INIT_THEME_DATA(CheckBox);
        INIT_THEME_DATA(ComboBox);
        INIT_THEME_DATA(ElevatedButton);
        INIT_THEME_DATA(FilledButton);
        INIT_THEME_DATA(FlatButton);
        INIT_THEME_DATA(Label);
        INIT_THEME_DATA(LabelArea);
        INIT_THEME_DATA(Layout);
        INIT_THEME_DATA(MenuItem);
        INIT_THEME_DATA(MenuSeparator);
        INIT_THEME_DATA(OnOffSwitch);
        INIT_THEME_DATA(OutlinedButton);
        INIT_THEME_DATA(PopupMenu);
        INIT_THEME_DATA(RawTextInput);
        INIT_THEME_DATA(ResizablePanel);
        INIT_THEME_DATA(ScrollView);
        INIT_THEME_DATA(Slider);
        INIT_THEME_DATA(TabCaption);
        INIT_THEME_DATA(TabGroup);
        INIT_THEME_DATA(TextInput);
        INIT_THEME_DATA(ToggleButton);
        INIT_THEME_DATA(TreeViewItem);
        INIT_THEME_DATA(ViewItem);
        INIT_THEME_DATA(Window);

#undef INIT_THEME_DATA
    }

    // Refer to https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/color#accent-color-palette

    D2D1_COLOR_F g_color1 = {};
    D2D1_COLOR_F g_color2 = {};
    D2D1_COLOR_F g_color3 = {};

    const D2D1_COLOR_F& color1()
    {
        return g_color1;
    }
    const D2D1_COLOR_F& color2()
    {
        return g_color2;
    }
    const D2D1_COLOR_F& color3()
    {
        return g_color3;
    }

    void generateTonedColors(const Appearance::ThemeStyle& style)
    {
        // TODO: Implement dynamic toned color generation algorithm.

        std::vector<BYTE> data = {}; DWORD dataSize = {};
        
        THROW_IF_FAILED(RegGetValue
        (
        /* hkey     */ HKEY_CURRENT_USER,
        /* lpSubKey */ L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent",
        /* lpValue  */ L"AccentPalette",
        /* dwFlags  */ RRF_RT_REG_BINARY,
        /* pdwType  */ nullptr,
        /* pvData   */ nullptr,
        /* pcbData  */ &dataSize
        ));
        THROW_IF_FALSE(dataSize == 32);
        data.resize(dataSize);

        THROW_IF_FAILED(RegGetValue
        (
        /* hkey     */ HKEY_CURRENT_USER,
        /* lpSubKey */ L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent",
        /* lpValue  */ L"AccentPalette",
        /* dwFlags  */ RRF_RT_REG_BINARY,
        /* pdwType  */ nullptr,
        /* pvData   */ data.data(),
        /* pcbData  */ &dataSize
        ));
        auto extractColor = [&](size_t index)
        {
            return D2D1::ColorF
            {
                (UINT32)
                (
                /* r */ data[4 * index + 0] << 16 |
                /* g */ data[4 * index + 1] << 8  |
                /* b */ data[4 * index + 2] << 0
                )
            };
        };
        // data[32] (8 RGBA colors):
        // AccentLight3 AccentLight2 AccentLight1 AccentNormal AccentDark1 AccentDark2 AccentDark3 Unknown
        // It is currently unclear what getAbgrFromRbga(7) [Unknown] represents.
        if (style.name == L"Light")
        {
            g_color1 = extractColor(4);
            g_color2 = extractColor(3);
            g_color3 = extractColor(2);
        }
        else if (style.name == L"Dark")
        {
            g_color1 = extractColor(1);
            g_color2 = extractColor(2);
            g_color3 = extractColor(3);
        }
    }
}
