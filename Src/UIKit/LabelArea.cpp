﻿#include "Common/Precompile.h"

#include "UIKit/LabelArea.h"

#include "Common/RuntimeError.h"

#include "Renderer/TickTimer.h"

#include "UIKit/Application.h"
#include "UIKit/Cursor.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    LabelArea::LabelArea(WstrRefer text, const D2D_RECT_F& rect)
        :
        Label(text, rect)
    {
        // Here left blank intentionally.
    }

    const Label::CharacterRange& LabelArea::hiliteRange() const
    {
        return m_hiliteRange;
    }

    void LabelArea::setHiliteRange(const Label::CharacterRange& range)
    {
        m_hiliteRange.offset = (size_t)std::clamp
        (
            (int)range.offset, 0, std::max((int)m_text.size() - 1, 0)
        );
        m_hiliteRange.count = (size_t)std::clamp // Cast size_t to int to avoid underflow.
        (
            (int)range.count, 0, std::max((int)m_text.size() - (int)m_hiliteRange.offset, 0)
        );
        appearance().indicator.visibility = (m_hiliteRange.count == 0);

        m_hiliteRangeData = hitTestTextRange(
            (UINT32)m_hiliteRange.offset, (UINT32)m_hiliteRange.count, 0.0f, 0.0f);
    }

    size_t LabelArea::hitTestCharacterOffset(const D2D1_POINT_2F& sfpt)
    {
        auto result = hitTestPoint(sfpt.x, sfpt.y);
        auto characterOffset = (size_t)result.metrics.textPosition;
        return result.isTrailingHit ? (characterOffset + 1) : characterOffset;
    }

    size_t LabelArea::indicatorPosition() const
    {
        return m_indicatorCharacterOffset;
    }

    void LabelArea::setIndicatorPosition(size_t characterOffset)
    {
        m_indicatorCharacterOffset = std::clamp(characterOffset, 0_uz, m_text.size());

        auto result = hitTestTextPos((UINT32)m_indicatorCharacterOffset, false);
        m_indicatorGeometry.first = { result.pointX, result.pointY };
        m_indicatorGeometry.second = { result.pointX, result.pointY + result.metrics.height };
    }

    void LabelArea::performCommandCtrlA()
    {
        setHiliteRange({ 0, m_text.size() });
        setIndicatorPosition(m_text.size());
    }

    void LabelArea::performCommandCtrlC()
    {
        auto hiliteStr = m_text.substr(m_hiliteRange.offset, m_hiliteRange.count);
        resource_utils::setClipboardText(hiliteStr);
    }

    void LabelArea::onRendererUpdateObject2DHelper(Renderer* rndr)
    {
        auto& animSetting = appearance().indicator.animation;

        auto deltaSecs = (float)rndr->timer()->deltaSecs();
        float blinkPeriod = animSetting.periodInSecs.blink;

        /////////////////////
        // Blink Indicator //
        /////////////////////

        if (holdKeyboardFocus() && (m_indicatorBlinkElapsedSecs += deltaSecs) >= blinkPeriod)
        {
            m_showIndicator = !m_showIndicator;
            m_indicatorBlinkElapsedSecs = 0.0f;
        }
    }

    void LabelArea::drawHiliteRange(Renderer* rndr)
    {
        auto& setting = appearance().hiliteRange;

        for (auto& rect : m_hiliteRangeData.metrics)
        {
            auto hiliteRangeRect = math_utils::roundf(selfCoordToAbsolute(
                math_utils::rect(rect.left, rect.top, rect.width, rect.height)));

            ////////////////
            // Background //
            ////////////////

            resource_utils::solidColorBrush()->SetColor(setting.background.color);
            resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

            rndr->d2d1DeviceContext()->FillRectangle
            (
            /* rect  */ hiliteRangeRect,
            /* brush */ resource_utils::solidColorBrush()
            );

            /////////////
            // Outline //
            /////////////

            resource_utils::solidColorBrush()->SetColor(setting.stroke.color);
            resource_utils::solidColorBrush()->SetOpacity(setting.stroke.opacity);

            float strokeWidth = setting.stroke.width;

            auto frame = math_utils::inner(hiliteRangeRect, strokeWidth);
            D2D1_ROUNDED_RECT outlineRect = { frame, roundRadiusX, roundRadiusY };

            rndr->d2d1DeviceContext()->DrawRoundedRectangle
            (
            /* roundedRect */ outlineRect,
            /* brush       */ resource_utils::solidColorBrush(),
            /* strokeWidth */ strokeWidth
            );
        }
    }

    void LabelArea::drawIndicator(Renderer* rndr)
    {
        auto& setting = appearance().indicator;

        if (setting.visibility && m_showIndicator)
        {
            auto result0 = math_utils::constrainToY(
                m_indicatorGeometry.first, indicatorConstrainedRect);

            auto result1 = math_utils::constrainToY(
                m_indicatorGeometry.second, indicatorConstrainedRect);

            if (result0.isOverlapped || result1.isOverlapped)
            {
                resource_utils::solidColorBrush()->SetColor(setting.background.color);
                resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

                result0.point = math_utils::offset(math_utils::roundf(
                    selfCoordToAbsolute(result0.point)), { 0.5f, 0.0f });

                result1.point = math_utils::offset(math_utils::roundf(
                    selfCoordToAbsolute(result1.point)), { 0.5f, 0.0f });

                rndr->d2d1DeviceContext()->DrawLine
                (
                /* point0 */ result0.point,
                /* point1 */ result1.point,
                /* brush  */ resource_utils::solidColorBrush()
                );
            }
        }
    }

    void LabelArea::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        drawBackground(rndr);
        drawHiliteRange(rndr);
        drawText(rndr);
        drawIndicator(rndr);
        drawOutline(rndr);
    }

    void LabelArea::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Label::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void LabelArea::onGetKeyboardFocusHelper()
    {
        Label::onGetKeyboardFocusHelper();

        m_showIndicator = true;
        m_indicatorBlinkElapsedSecs = 0.0f;

        increaseAnimationCount();
    }

    void LabelArea::onLoseKeyboardFocusHelper()
    {
        Label::onLoseKeyboardFocusHelper();

        if (!keepHiliteRange)
        {
            setHiliteRange({ 0, 0 });
        }
        m_showIndicator = false;
        m_indicatorBlinkElapsedSecs = 0.0f;

        if (!keepIndicatorPosition)
        {
            setIndicatorPosition(0);
        }
        decreaseAnimationCount();
    }

    void LabelArea::onMouseMoveHelper(MouseMoveEvent& e)
    {
        Label::onMouseMoveHelper(e);

        THROW_IF_NULL(Application::g_app);

        Application::g_app->cursor()->setIcon(Cursor::Text);

        if (holdKeyboardFocus() && e.buttonState.leftPressed)
        {
            m_showIndicator = true;
            m_indicatorBlinkElapsedSecs = 0.0f;

            auto characterOffset = hitTestCharacterOffset(absoluteToSelfCoord(e.cursorPoint));

            setHiliteRange(
            {
                std::min(characterOffset, m_hiliteRangeOrigin),
                (size_t)std::abs((int)characterOffset - (int)m_hiliteRangeOrigin)
            });
            setIndicatorPosition(characterOffset);
        }
    }

    void LabelArea::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Label::onMouseButtonHelper(e);

        THROW_IF_NULL(Application::g_app);

        if (e.state.leftDown() || e.state.leftDblclk())
        {
            Application::g_app->focusUIObject
            (
                Application::FocusType::Keyboard, shared_from_this()
            );
            m_showIndicator = true;
            m_indicatorBlinkElapsedSecs = 0.0f;

            m_hiliteRangeOrigin = hitTestCharacterOffset(absoluteToSelfCoord(e.cursorPoint));

            setHiliteRange({ 0, 0 });

            setIndicatorPosition(m_hiliteRangeOrigin);
        }
    }

    void LabelArea::onKeyboardHelper(KeyboardEvent& e)
    {
        Label::onKeyboardHelper(e);

        if (holdKeyboardFocus() && e.state.pressed() && e.CTRL())
        {
            switch (e.vkey)
            {
            case 'A': performCommandCtrlA(); break;
            case 'C': performCommandCtrlC(); break;
            default: break;
            }
        }
    }
}
