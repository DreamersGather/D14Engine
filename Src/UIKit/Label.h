﻿#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/Label.h"
#include "UIKit/Panel.h"

namespace d14engine::uikit
{
    struct Label : appearance::Label, Panel
    {
        Label(WstrRefer text = {}, const D2D_RECT_F& rect = {});

        _D14_SET_APPEARANCE_PROPERTY(Label)

        static Wstring defaultTextFormatName;

    protected:
        Wstring m_text = {};

        // input-str ---> preprocess ---> output-str ---> set-text
        virtual Optional<Wstring> preprocessInputStr(WstrRefer in);

    public:
        const Wstring& text() const;
        virtual void setText(WstrRefer text);

        void setTextFormat(IDWriteTextFormat* textFormat);

        void insertTextFragment(WstrRefer fragment, size_t offset);
        void appendTextFragment(WstrRefer fragment);

        struct CharacterRange { size_t offset, count; };

        void eraseTextFragment(const CharacterRange& range);

        void copyTextStyle(Label* source, OptRefer<WstringView> text = {});

    protected:
        ComPtr<IDWriteTextLayout> m_textLayout = {};

        DWRITE_OVERHANG_METRICS m_textOverhangs = {};

    public:
        IDWriteTextLayout* textLayout() const;

        DWRITE_TEXT_METRICS textMetrics() const;

        const DWRITE_OVERHANG_METRICS& textOverhangs() const;
        void updateTextOverhangMetrics();

        D2D1_SIZE_F textAreaSize() const;

        enum class HorzAlignment { Left, Right, Center, None };
        enum class VertAlignment { Top, Bottom, Center, None };

#define CONST_ENUM constexpr static auto

        CONST_ENUM Left = HorzAlignment::Left;
        CONST_ENUM Right = HorzAlignment::Right;
        CONST_ENUM HCenter = HorzAlignment::Center;
        CONST_ENUM HNone = HorzAlignment::None;
        CONST_ENUM Top = VertAlignment::Top;
        CONST_ENUM Bottom = VertAlignment::Bottom;
        CONST_ENUM VCenter = VertAlignment::Center;
        CONST_ENUM VNone = VertAlignment::None;

#undef CONST_ENUM

        // Uses the cached overhang metrics to force align the text.
        struct HardAlignment
        {
            HorzAlignment horz = HorzAlignment::None;
            VertAlignment vert = VertAlignment::None;
        }
        hardAlignment = {}; // compare this with TextLayout.Alignment

        struct TextLayoutParams
        {
            Optional<WstringView> text = {};

            IDWriteTextFormat* textFormat = nullptr;

            Optional<float> maxWidth = {};
            Optional<float> maxHeight = {};

            Optional<float> incrementalTabStop = {};
            Optional<DWRITE_TEXT_ALIGNMENT> textAlignment = {};
            Optional<DWRITE_PARAGRAPH_ALIGNMENT> paragraphAlignment = {};
            Optional<DWRITE_WORD_WRAPPING> wordWrapping = {};
        };
        ComPtr<IDWriteTextLayout>
        getTextLayout(const TextLayoutParams& params = {}) const;

        using TextMetricsParams = TextLayoutParams;

        DWRITE_TEXT_METRICS // generate new temporary layout
        getTextMetrics(const TextMetricsParams& params = {}) const;

    public:
        D2D1_DRAW_TEXT_OPTIONS drawTextOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;

        struct PointHitTestResult
        {
            BOOL isTrailingHit = {};
            BOOL isInside = {};
            DWRITE_HIT_TEST_METRICS metrics = {};
        };
        PointHitTestResult hitTestPoint(FLOAT pointX, FLOAT pointY);

        struct TextPosHitTestResult
        {
            FLOAT pointX = {};
            FLOAT pointY = {};
            DWRITE_HIT_TEST_METRICS metrics = {};
        };
        TextPosHitTestResult hitTestTextPos(UINT32 textPosition, BOOL isTrailingHit);

        struct TextRangeHitTestResult
        {
            TextRangeHitTestResult(UINT32 count = 0)
                :
                metrics(count) { }

            std::vector<DWRITE_HIT_TEST_METRICS> metrics = {};
        };
        TextRangeHitTestResult // perform hit test for multiline
        hitTestTextRange(UINT32 textPosition, UINT32 textLength, FLOAT originX, FLOAT originY);

    protected:
        // IDrawObject2D
        void drawBackground(renderer::Renderer* rndr);
        void drawText(renderer::Renderer* rndr);
        void drawOutline(renderer::Renderer* rndr);
        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        // Panel
        void onSizeHelper(SizeEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;
    };
}
