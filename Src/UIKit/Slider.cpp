#include "Common/Precompile.h"

#include "UIKit/Slider.h"

#include "Common/DirectXError.h"
#include "Common/MathUtils/Basic.h"

#include "UIKit/Application.h"
#include "UIKit/Label.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    Slider::Slider(
        const D2D1_RECT_F& rect,
        float value,
        float minValue,
        float maxValue)
        :
        Panel(rect, resource_utils::solidColorBrush())
    {
        appEventReactability.focus.get = true;

        m_takeOverChildrenDrawing = true;

        m_value = value;
        m_minValue = minValue;
        m_maxValue = maxValue;

        m_valueLabel = makeUIObject<Label>();

        m_valueLabel->setPrivateVisible(false);
        m_valueLabel->setPrivateEnabled(false);

        m_valueLabel->setTextFormat(D14_FONT(L"Default/9"));
        THROW_IF_FAILED(m_valueLabel->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    }

    void Slider::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        addUIObject(m_valueLabel);

        m_valueLabel->transform(valueLabelSelfCoordRect());

        std::wstringstream ss = {};

        auto& precision = getAppearance().valueLabel.precision;
        ss << std::fixed << std::setprecision(precision) << m_value;

        m_valueLabel->setText(ss.str());

        loadHandleShadowBitmap();
        loadValueLabelMaskBitmap();

        loadSideTrianglePathGeo();
    }

    void Slider::onStartSliding(float value)
    {
        onStartSlidingHelper(value);

        if (f_onStartSliding) f_onStartSliding(this, value);
    }

    void Slider::onEndSliding(float value)
    {
        onEndSlidingHelper(value);

        if (f_onEndSliding) f_onEndSliding(this, value);
    }

    void Slider::onStartSlidingHelper(float value)
    {
        forceGlobalExclusiveFocusing = true;
    }

    void Slider::onEndSlidingHelper(float value)
    {
        forceGlobalExclusiveFocusing = false;
    }

    void Slider::loadHandleShadowBitmap()
    {
        auto& geoSetting = getAppearance().handle.geometry;

        handleShadow.loadBitmap(geoSetting.size);
    }

    void Slider::loadSideTrianglePathGeo()
    {
        auto factory = Application::g_app->dx12Renderer()->d2d1Factory();
        THROW_IF_FAILED(factory->CreatePathGeometry(&sideTrianglePathGeo));

        ComPtr<ID2D1GeometrySink> geoSink = {};
        THROW_IF_FAILED(sideTrianglePathGeo->Open(&geoSink));
        {
            auto triangleVertices = valueLabelSideTriangleInShadow();

            geoSink->BeginFigure(triangleVertices[2], D2D1_FIGURE_BEGIN_FILLED);

            geoSink->AddLines(triangleVertices.data(), (UINT32)triangleVertices.size());

            geoSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        }
        THROW_IF_FAILED(geoSink->Close());
    }

    bool Slider::setValue(float value)
    {
        // ValuefulObject::setValue rejects the value out of range.
        value = std::clamp(value, m_minValue, m_maxValue);

        bool isValueChanged = ValuefulObject::setValue(value);
        if (isValueChanged) onValueChange(m_value);
        return isValueChanged;
    }

    bool Slider::setMinValue(float value)
    {
        bool isValueChanged = ValuefulObject::setMinValue(value);
        if (isValueChanged) onValueChange(m_value);
        return isValueChanged;
    }

    bool Slider::setMaxValue(float value)
    {
        bool isValueChanged = ValuefulObject::setMaxValue(value);
        if (isValueChanged) onValueChange(m_value);
        return isValueChanged;
    }

    const SharedPtr<Label>& Slider::valueLabel() const
    {
        return m_valueLabel;
    }

    void Slider::updateValue(const D2D1_POINT_2F& offset)
    {
        float tmpValue = geometryOffsetToValue(offset);

        if (stepMode == StepMode::Discrete && stepInterval > 0.0f)
        {
            float diffValue = tmpValue - m_value;
            float absDiffValue = std::abs(diffValue);

            if (absDiffValue + FLT_EPSILON >= stepInterval)
            {
                setValue(m_value + math_utils::round(diffValue / stepInterval) * stepInterval);
            }
        }
        else setValue(tmpValue);
    }

    void Slider::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        ///////////////////
        // Handle Shadow //
        ///////////////////

        handleShadow.beginDraw(rndr->d2d1DeviceContext());
        {
            auto& setting = getAppearance().handle;

            resource_utils::solidColorBrush()->SetOpacity(1.0f);

            auto handleRect = math_utils::rect
            (
                { 0.0f, 0.0f }, setting.geometry.size
            );
            D2D1_ROUNDED_RECT roundedRect =
            {
                math_utils::moveVertex(handleRect, setting.shadow.offset),
                setting.geometry.roundRadius, setting.geometry.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ roundedRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        handleShadow.endDraw(rndr->d2d1DeviceContext());

        //////////////////////
        // Value Label Mask //
        //////////////////////

        if (m_valueLabel->isD2d1ObjectVisible())
        {
            valueLabelMask.beginDraw(rndr->d2d1DeviceContext());
            {
                auto& setting = getAppearance().valueLabel;

                //----------------------------------------------------------
                // Main rect
                //----------------------------------------------------------

                auto& rectBkgn = setting.mainRect.background;

                resource_utils::solidColorBrush()->SetColor(rectBkgn.color);
                resource_utils::solidColorBrush()->SetOpacity(rectBkgn.opacity);
                
                D2D1_ROUNDED_RECT roundedRect =
                {
                    valueLabelMainRectInShadow(),
                    setting.mainRect.geometry.roundRadius,
                    setting.mainRect.geometry.roundRadius
                };
                rndr->d2d1DeviceContext()->FillRoundedRectangle
                (
                /* roundedRect */ roundedRect,
                /* brush       */ resource_utils::solidColorBrush()
                );
                //----------------------------------------------------------
                // Side Triangle
                //----------------------------------------------------------

                auto& trngBkgn = setting.sideTriangle.background;

                resource_utils::solidColorBrush()->SetColor(trngBkgn.color);
                resource_utils::solidColorBrush()->SetOpacity(trngBkgn.opacity);

                rndr->d2d1DeviceContext()->FillGeometry
                (
                /* geometry */ sideTrianglePathGeo.Get(),
                /* brush    */ resource_utils::solidColorBrush()
                );
            }
            valueLabelMask.endDraw(rndr->d2d1DeviceContext());
        }
    }

    void Slider::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        //////////////////
        // Complete Bar //
        //////////////////

        auto& complete = getAppearance().bar.complete;
        auto& completeBkgn = m_enabled ? complete.background : complete.secondaryBackground;

        resource_utils::solidColorBrush()->SetColor(completeBkgn.color);
        resource_utils::solidColorBrush()->SetOpacity(completeBkgn.opacity);

        D2D1_ROUNDED_RECT completeRect =
        {
            completeBarAbsoluteRect(),
            complete.geometry.roundRadius,
            complete.geometry.roundRadius
        };
        rndr->d2d1DeviceContext()->FillRoundedRectangle
        (
        /* roundedRect */ completeRect,
        /* brush       */ resource_utils::solidColorBrush()
        );

        ////////////////
        // Filled Bar //
        ////////////////

        auto& filled = getAppearance().bar.filled;
        auto& filledBkgn = m_enabled ? filled.background : filled.secondaryBackground;

        resource_utils::solidColorBrush()->SetColor(filledBkgn.color);
        resource_utils::solidColorBrush()->SetOpacity(filledBkgn.opacity);

        D2D1_ROUNDED_RECT filledRect =
        {
            filledBarAbsoluteRect(),
            filled.geometry.roundRadius,
            filled.geometry.roundRadius
        };
        rndr->d2d1DeviceContext()->FillRoundedRectangle
        (
        /* roundedRect */ filledRect,
        /* brush       */ resource_utils::solidColorBrush()
        );

        ////////////
        // Handle //
        ////////////
        {
            //-------------------------------------------------------------------------
            // Shadow
            //-------------------------------------------------------------------------

            auto& shadow = getAppearance().handle.shadow;
            handleShadow.color = m_enabled ? shadow.color : shadow.secondaryColor;

            handleShadow.configEffectInput(resource_utils::shadowEffect());

            rndr->d2d1DeviceContext()->DrawImage
            (
            /* effect       */ resource_utils::shadowEffect(),
            /* targetOffset */ math_utils::leftTop(handleAbsoluteRect())
            );
            //-------------------------------------------------------------------------
            // Entity
            //-------------------------------------------------------------------------

            auto& handle = getAppearance().handle;
            auto& handleBkgn = m_enabled ? handle.background : handle.secondaryBackground;

            resource_utils::solidColorBrush()->SetColor(handleBkgn.color);
            resource_utils::solidColorBrush()->SetOpacity(handleBkgn.opacity);

            D2D1_ROUNDED_RECT roundedRect =
            {
                handleAbsoluteRect(),
                handle.geometry.roundRadius,
                handle.geometry.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ roundedRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        /////////////////
        // Value Label //
        /////////////////

        if (getAppearance().valueLabel.isResident)
        {
            m_valueLabel->setPrivateVisible(true);
            m_valueLabel->setPrivateEnabled(true);
        }
        if (m_valueLabel->isD2d1ObjectVisible())
        {
            auto vlblRect = selfCoordToAbsolute(valueLabelShadowSelfCoordRect());

            //-------------------------------------------------------------------------
            // Shadow
            //-------------------------------------------------------------------------

            auto& shadowSetting = getAppearance().valueLabel.shadow;

            valueLabelMask.color = shadowSetting.color;
            valueLabelMask.standardDeviation = shadowSetting.standardDeviation;

            valueLabelMask.configEffectInput(resource_utils::shadowEffect());

            rndr->d2d1DeviceContext()->DrawImage
            (
            /* effect       */ resource_utils::shadowEffect(),
            /* targetOffset */ math_utils::leftTop(vlblRect)
            );
            //-------------------------------------------------------------------------
            // Entity
            //-------------------------------------------------------------------------

            rndr->d2d1DeviceContext()->DrawBitmap
            (
            /* bitmap               */ valueLabelMask.data.Get(),
            /* destinationRectangle */ vlblRect,
            /* opacity              */ valueLabelMask.opacity,
            /* interpolationMode    */ valueLabelMask.getInterpolationMode()
            );
            //-------------------------------------------------------------------------
            // Text
            //-------------------------------------------------------------------------

            m_valueLabel->onRendererDrawD2d1Object(rndr);
        }
    }

    bool Slider::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isOverlapped(p, thumbAreaRect(m_absoluteRect));
    }

    void Slider::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        m_valueLabel->transform(valueLabelSelfCoordRect());

        loadHandleShadowBitmap();
        loadValueLabelMaskBitmap();
    }

    void Slider::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Panel::onChangeThemeStyleHelper(style);

        getAppearance().changeTheme(style.name);
    }

    void Slider::onMouseMoveHelper(MouseMoveEvent& e)
    {
        Panel::onMouseMoveHelper(e);

        if (m_isSliding) updateValue(absoluteToSelfCoord(e.cursorPoint));
    }

    void Slider::onMouseButtonHelper(MouseButtonEvent& e)
    {
        Panel::onMouseButtonHelper(e);

        auto& p = e.cursorPoint;

        if (e.state.leftDown() || e.state.leftDblclk())
        {
            m_isSliding = true;

            updateValue(absoluteToSelfCoord(p));

            if (getAppearance().valueLabel.enabled)
            {
                m_valueLabel->setPrivateVisible(true);
                m_valueLabel->setPrivateEnabled(true);
            }
            onStartSliding(m_value);
        }
        else if (e.state.leftUp())
        {
            if (m_isSliding)
            {
                m_isSliding = false;

                if (getAppearance().valueLabel.enabled)
                {
                    m_valueLabel->setPrivateVisible(false);
                    m_valueLabel->setPrivateEnabled(false);
                }
                onEndSliding(m_value);
            }
        }
    }

    void Slider::onValueChangeHelper(float value)
    {
        ValuefulObject::onValueChangeHelper(value);

        m_valueLabel->transform(valueLabelSelfCoordRect());

        std::wstringstream ss;

        auto& precision = getAppearance().valueLabel.precision;
        ss << std::fixed << std::setprecision(precision) << value;

        m_valueLabel->setText(ss.str());
    }
}
