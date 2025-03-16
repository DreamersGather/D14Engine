#pragma once

#include "Common/Precompile.h"

namespace d14engine::uikit::resource_utils
{
    void initialize();

#pragma region Font

    struct FontDetail
    {
        Wstring family = {};
        Wstring locale = {};
    };
    struct FontDetailLess
    {
        bool operator()(const FontDetail& lhs, const FontDetail& rhs) const
        {
            return std::tie(lhs.family, lhs.locale) < std::tie(rhs.family, rhs.locale);
        }
    };
    using FontDetailSet = std::set<FontDetail, FontDetailLess>;

    FontDetailSet querySystemFonts();

    // Call this with query=True to force updating the cached set.
    // In most cases, there is no need to use the parameter because
    // the application will perform an immediate query at startup and
    // cache the returned result. If the user installs a new font
    // while the application is running, set query=True to refresh.
    const FontDetailSet& systemFonts(bool query = false);

    using TextFormatMap = std::unordered_map<Wstring, ComPtr<IDWriteTextFormat>>;

    const TextFormatMap& textFormats();

    struct TextFormatDetail
    {
        FontDetail font = {}; FLOAT size = {};
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
    };
    // Performance tip: When this function is called, the loaded text format will be cached
    // in the application's global resource set (g_textFormats) with the specified name.
    // Therefore, the next time the same text format is needed, it can be directly queried
    // and obtained through textFormats(), eliminating the repeated loading of font data.
    ComPtr<IDWriteTextFormat> loadTextFormat(WstrRefer name, const TextFormatDetail& detail);

    void loadBasicTextFormats();

#define D14_FONT(Name) d14engine::uikit::resource_utils::textFormats().at(Name).Get()

#pragma endregion

#pragma region Common

    ID2D1SolidColorBrush* solidColorBrush();

    void loadCommonBrushes();

    ID2D1Effect* shadowEffect();

    void loadCommonEffects();

#pragma endregion

#pragma region Clipboard

    Optional<Wstring> getClipboardText(HWND hWndNewOwner = nullptr);

    void setClipboardText(WstrRefer content, HWND hWndNewOwner = nullptr);

#pragma endregion

#pragma region Keyboard Layout

    struct PrintableChar
    {
        WCHAR normal = {};
        // Pick the alternative char when Shift is pressed.
        WCHAR alternative = {};
    };
    using VirtualKeyCode = int;

    using KeyboardLayoutMap = std::unordered_map<VirtualKeyCode, PrintableChar>;

    const KeyboardLayoutMap& keyboardLayout(); // standard keyboard layout

#pragma endregion

}
