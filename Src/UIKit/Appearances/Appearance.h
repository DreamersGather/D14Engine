#pragma once

#include "Common/Precompile.h"

namespace d14engine::uikit::appearance
{
    void initialize();

#pragma region Interfaces

    struct Appearance
    {
        struct ThemeStyle
        {
            Wstring name = {}; D2D1_COLOR_F color = {};

            bool operator==(const ThemeStyle& rhs)
            {
                return name == rhs.name &&
                    color.r == rhs.color.r &&
                    color.g == rhs.color.g &&
                    color.b == rhs.color.b &&
                    color.a == rhs.color.a;
            }
        };
        virtual void changeTheme(WstrRefer themeName) = 0;
    };

#define _D14_SET_APPEARANCE_PROPERTY(Type_Name) \
    appearance::Type_Name::Appearance& appearance() \
    { \
        return appearance::Type_Name::appearanceData; \
    } \
    const appearance::Type_Name::Appearance& appearance() const \
    { \
        return appearance::Type_Name::appearanceData; \
    }

#define _D14_SET_THEME_DATA_MAP_DECL \
    using ThemeDataMap = std::unordered_map<Wstring, ThemeData>; \
    static ThemeDataMap g_themeData;

#define _D14_REF_THEME_DATA_MAP_DECL(Type_Name) \
    using ThemeData = Type_Name::Appearance::ThemeData; \
    using ThemeDataMap = Type_Name::Appearance::ThemeDataMap; \
    static ThemeDataMap g_themeData;

#define _D14_SET_THEME_DATA_MAP_IMPL(Type_Name) \
    Type_Name::Appearance::ThemeDataMap Type_Name::Appearance::g_themeData = {};

    // Consider the following scenario:
    //
    //     student <--- student_appear
    //        |
    // fashion_student <--- fashion_student_appear <--- student_appear
    //
    // which is a typical diamond problem and can be solved with virtual
    // inheritance, but here we use another way (i.e. proxy-method):
    //
    //     student <--- student_appear ---> (pass reference) ----->
    //        |                                                   |
    // fashion_student <--- fashion_student_appear <--- student_appear_proxy
    //
    // The reason for not using multiple virtual inheritance here is that
    // it has already been used in the construction of the UI object tree,
    // and UI objects also need to inherit their related Appearance objects.
    // If multiple virtual inheritance were used for Appearance as well,
    // it would make the entire OOP framework much more complex and tricky.

    template<typename T>
    struct AppearanceProxy
    {
        using Master = T;

        static_assert(std::is_base_of_v<Appearance, Master>,
            "AppearanceProxy::Master must inherit from Appearance");

        virtual void changeTheme(Master& m, WstrRefer themeName) = 0;
    };

#pragma endregion

#pragma region Color Palette

// Refer to https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/color#accent-color-palette

    const D2D1_COLOR_F& color1();
    const D2D1_COLOR_F& color2();
    const D2D1_COLOR_F& color3();

    namespace color
    {
        const D2D1_COLOR_F& normal();

        const D2D1_COLOR_F& light1();
        const D2D1_COLOR_F& light2();
        const D2D1_COLOR_F& light3();

        const D2D1_COLOR_F& dark1();
        const D2D1_COLOR_F& dark2();
        const D2D1_COLOR_F& dark3();
    }
    // Call Appearance::changeTheme after generated new colors.
    void generateTonedColors(const Appearance::ThemeStyle& style);

#pragma endregion

#pragma region Macro Helpers

#define _D14_FIND_THEME_DATA(Theme_Name) \
    auto _itor = g_themeData.find(Theme_Name); \
    if (_itor == g_themeData.end()) return; \
    auto& _ref = _itor->second

#define _D14_UPDATE_THEME_DATA_1(Data_Name) \
    Data_Name = _ref.Data_Name

#define _D14_UPDATE_THEME_DATA_2(Data_Name) \
    m.Data_Name = _ref.Data_Name

#define _D14_UPDATE_THEME_DATA_ARRAY_1(Data_Name) \
    memcpy(Data_Name, _ref.Data_Name, sizeof(_ref.Data_Name))

#define _D14_UPDATE_THEME_DATA_ARRAY_2(Data_Name) \
    memcpy(m.Data_Name, _ref.Data_Name, sizeof(_ref.Data_Name))

#pragma endregion

}
