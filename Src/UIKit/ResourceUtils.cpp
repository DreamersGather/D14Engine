#include "Common/Precompile.h"

#include "UIKit/ResourceUtils.h"

#include "Common/DirectXError.h"

#include "UIKit/Application.h"

namespace d14engine::uikit::resource_utils
{
    void initialize()
    {
        systemFonts(true);
        loadBasicTextFormats();

        loadCommonBrushes();
        loadCommonEffects();
    }

    FontDetailSet querySystemFonts()
    {
        THROW_IF_NULL(Application::g_app);

        FontDetailSet fonts = {};

        auto factory = Application::g_app->dx12Renderer()->dwriteFactory();

        /////////////////////
        // Font Collection //
        /////////////////////

        ComPtr<IDWriteFontCollection1> collection = {};
        THROW_IF_FAILED(factory->GetSystemFontCollection(FALSE, &collection, TRUE));

        ///////////////////
        // Font Families //
        ///////////////////

        auto familyCount = collection->GetFontFamilyCount();
        std::vector<ComPtr<IDWriteFontFamily>> families(familyCount);

        for (UINT32 i = 0; i < families.size(); ++i)
        {
            THROW_IF_FAILED(collection->GetFontFamily(i, &families[i]));
        }

        //////////////////
        // Font Details //
        //////////////////

        for (auto& f : families)
        {
            ComPtr<IDWriteLocalizedStrings> names = {};
            THROW_IF_FAILED(f->GetFamilyNames(&names));

            auto count = names->GetCount();
            for (UINT32 i = 0; i < count; ++i)
            {
                UINT32 familyLen = {}, localeLen = {};

                THROW_IF_FAILED(names->GetStringLength(i, &familyLen));
                THROW_IF_FAILED(names->GetLocaleNameLength(i, &localeLen));

                Wstring family(familyLen, (WCHAR)0);
                Wstring locale(localeLen, (WCHAR)0);

                // The size must include space for the terminating null character.
                THROW_IF_FAILED(names->GetString(i, family.data(), familyLen + 1));
                THROW_IF_FAILED(names->GetLocaleName(i, locale.data(), localeLen + 1));

                fonts.insert({ family, locale });
            }
        }
        return fonts;
    }

    FontDetailSet g_systemFonts = {};

    const FontDetailSet& systemFonts(bool query)
    {
        return query ? (g_systemFonts = querySystemFonts()) : g_systemFonts;
    }

    TextFormatMap g_textFormats = {};

    const TextFormatMap& textFormats()
    {
        return g_textFormats;
    }

    ComPtr<IDWriteTextFormat> loadTextFormat
    (WstrRefer name, const TextFormatDetail& detail)
    {
        THROW_IF_NULL(Application::g_app);

        auto factory = Application::g_app->dx12Renderer()->dwriteFactory();

        auto& textFormat = (g_textFormats[name] = nullptr);

        // 1 inch == 72 pt == 96 dip
        THROW_IF_FAILED(factory->CreateTextFormat
        (
        /* fontFamilyName */ detail.font.family.c_str(),
        /* fontCollection */ nullptr,
        /* fontWeight     */ detail.weight,
        /* fontStyle      */ detail.style,
        /* fontStretch    */ detail.stretch,
        /* fontSize       */ detail.size * 96.0f / 72.0f,
        /* localeName     */ detail.font.locale.c_str(),
        /* textFormat     */ &textFormat
        ));
        return textFormat;
    }

    void loadBasicTextFormats()
    {
        for (int size = 9; size <= 36; ++size)
        {
            Wstring name =
            {
                L"Default/" + std::to_wstring(size)
            };
            TextFormatDetail detail =
            {
                .font =
                {
                    .family = L"Segoe UI",
                    .locale = L"en-us"
                },
                .size = (float)size
            };
            loadTextFormat(name, detail);
        }
    }

    ComPtr<ID2D1SolidColorBrush> g_solidColorBrush = {};

    ID2D1SolidColorBrush* solidColorBrush()
    {
        return g_solidColorBrush.Get();
    }

    void loadCommonBrushes()
    {
        THROW_IF_NULL(Application::g_app);

        auto context = Application::g_app->dx12Renderer()->d2d1DeviceContext();

        ///////////////////////
        // Solid Color Brush //
        ///////////////////////

        auto black = (D2D1_COLOR_F)D2D1::ColorF::Black;
        THROW_IF_FAILED(context->CreateSolidColorBrush(black, &g_solidColorBrush));
    }

    ComPtr<ID2D1Effect> g_shadowEffect = {};

    ID2D1Effect* shadowEffect()
    {
        return g_shadowEffect.Get();
    }

    void loadCommonEffects()
    {
        THROW_IF_NULL(Application::g_app);

        auto context = Application::g_app->dx12Renderer()->d2d1DeviceContext();

        ///////////////////
        // Shadow Effect //
        ///////////////////

        THROW_IF_FAILED(context->CreateEffect(CLSID_D2D1Shadow, &g_shadowEffect));
    }

    Optional<Wstring> getClipboardText(HWND hWndNewOwner)
    {
        Optional<Wstring> content = {};

        if (IsClipboardFormatAvailable(CF_UNICODETEXT))
        {
            if (OpenClipboard(hWndNewOwner))
            {
                auto hGlobal = GetClipboardData(CF_UNICODETEXT);
                if (hGlobal)
                {
                    auto pGlobal = (WCHAR*)GlobalLock(hGlobal);
                    if (pGlobal)
                    {
                        content = pGlobal;
                        GlobalUnlock(hGlobal);
                    }
                }
                CloseClipboard();
            }
        }
        return content;
    }

    void setClipboardText(WstrRefer content, HWND hWndNewOwner)
    {
        if (OpenClipboard(hWndNewOwner))
        {
            auto hSize = sizeof(WCHAR) * (content.size() + 1);
            auto hGlobal = GlobalAlloc(GMEM_MOVEABLE, hSize);
            if (hGlobal)
            {
                auto pGlobal = (WCHAR*)GlobalLock(hGlobal);
                if (pGlobal)
                {
                    auto pSize = sizeof(WCHAR) * content.size();
                    memcpy(pGlobal, content.data(), pSize);
                    pGlobal[content.size()] = (WCHAR)0;
                    GlobalUnlock(hGlobal);

                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                }
            }
            CloseClipboard();
        }
    }

    const static KeyboardLayoutMap g_keyboardLayout =
    {
        //////////////
        // Alphabet //
        //////////////

        { 0x41, { L'a', L'A' } },
        { 0x42, { L'b', L'B' } },
        { 0x43, { L'c', L'C' } },
        { 0x44, { L'd', L'D' } },
        { 0x45, { L'e', L'E' } },
        { 0x46, { L'f', L'F' } },
        { 0x47, { L'g', L'G' } },
        { 0x48, { L'h', L'H' } },
        { 0x49, { L'i', L'I' } },
        { 0x4A, { L'j', L'J' } },
        { 0x4B, { L'k', L'K' } },
        { 0x4C, { L'l', L'L' } },
        { 0x4D, { L'm', L'M' } },
        { 0x4E, { L'n', L'N' } },
        { 0x4F, { L'o', L'O' } },
        { 0x50, { L'p', L'P' } },
        { 0x51, { L'q', L'Q' } },
        { 0x52, { L'r', L'R' } },
        { 0x53, { L's', L'S' } },
        { 0x54, { L't', L'T' } },
        { 0x55, { L'u', L'U' } },
        { 0x56, { L'v', L'V' } },
        { 0x57, { L'w', L'W' } },
        { 0x58, { L'x', L'X' } },
        { 0x59, { L'y', L'Y' } },
        { 0x5A, { L'z', L'Z' } },

        /////////////
        // Numbers //
        /////////////

        { 0x30, { L'0', L')' } },
        { 0x31, { L'1', L'!' } },
        { 0x32, { L'2', L'@' } },
        { 0x33, { L'3', L'#' } },
        { 0x34, { L'4', L'$' } },
        { 0x35, { L'5', L'%' } },
        { 0x36, { L'6', L'^' } },
        { 0x37, { L'7', L'&' } },
        { 0x38, { L'8', L'*' } },
        { 0x39, { L'9', L'(' } },

        /////////////
        // Symbols //
        /////////////

        { VK_OEM_1, { L';', L':' } },
        { VK_OEM_2, { L'/', L'?' } },
        { VK_OEM_3, { L'`', L'~' } },
        { VK_OEM_4, { L'[', L'{' } },
        { VK_OEM_5, { L'\\', L'|' } },
        { VK_OEM_6, { L']', L'}' } },
        { VK_OEM_7, { L'\'', L'"' } },

        ///////////////////
        // Miscellaneous //
        ///////////////////

        { VK_SPACE,      { L' ', L' ' } },
        { VK_OEM_PLUS,   { L'=', L'+' } },
        { VK_OEM_COMMA,  { L',', L'<' } },
        { VK_OEM_MINUS,  { L'-', L'_' } },
        { VK_OEM_PERIOD, { L'.', L'>' } },

        ////////////////////
        // NumPad Numbers //
        ////////////////////

        { VK_NUMPAD0, { L'0', L'0' } },
        { VK_NUMPAD1, { L'1', L'1' } },
        { VK_NUMPAD2, { L'2', L'2' } },
        { VK_NUMPAD3, { L'3', L'3' } },
        { VK_NUMPAD4, { L'4', L'4' } },
        { VK_NUMPAD5, { L'5', L'5' } },
        { VK_NUMPAD6, { L'6', L'6' } },
        { VK_NUMPAD7, { L'7', L'7' } },
        { VK_NUMPAD8, { L'8', L'8' } },
        { VK_NUMPAD9, { L'9', L'9' } },

        ////////////////////
        // NumPad Symbols //
        ////////////////////

        { VK_ADD,      { L'+', L'+' } },
        { VK_SUBTRACT, { L'-', L'-' } },
        { VK_MULTIPLY, { L'*', L'*' } },
        { VK_DIVIDE,   { L'/', L'/' } },
        { VK_DECIMAL,  { L'.', L'.' } },
    };

    const KeyboardLayoutMap& keyboardLayout()
    {
        return g_keyboardLayout;
    }
}
