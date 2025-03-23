#include "Common/Precompile.h"

#include "UIKit/Slider.h"

#include "Common/DirectXError.h"

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
        SliderBase(rect, value, minValue, maxValue)
    {
        // Here left blank intentionally.
    }

    void Slider::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        ///////////////////////////
        // Load Cached Resources //
        ///////////////////////////

        handleRes.loadShadow();

        sideTriangleRes.loadPathGeo();

        ///////////////////////////
        // Init Children Objects //
        ///////////////////////////

        m_valueLabel = makeUIObject<Label>();
        {
            registerUIEvents(m_valueLabel);

            m_valueLabel->setPrivateVisible(false);
            m_valueLabel->setPrivateEnabled(false);

            m_valueLabel->transform(valueLabelSelfCoordRect());

            m_valueLabel->setTextFormat(D14_FONT(L"Default/9"));

            auto layout = m_valueLabel->textLayout();
            auto alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
            THROW_IF_FAILED(layout->SetTextAlignment(alignment));

            std::wstringstream ss = {};

            auto& precision = appearance().valueLabel.precision;
            ss << std::fixed << std::setprecision(precision) << m_value;

            m_valueLabel->setText(ss.str());
        }
    }

    void Slider::HandleRes::loadShadow()
    {
        Slider* sldr = m_master;
        THROW_IF_NULL(sldr);

        auto& setting = sldr->appearance().handle;

        shadow.loadBitmap(setting.geometry.size);
    }

    MaskObject& Slider::ValueLabelRes::mask()
    {
        return shadowMask;
    }

    ShadowMask& Slider::ValueLabelRes::shadow()
    {
        return shadowMask;
    }

    void Slider::SideTriangleRes::loadPathGeo()
    {
        Slider* sldr = m_master;
        THROW_IF_NULL(sldr);

        THROW_IF_NULL(Application::g_app);

        auto factory = Application::g_app->renderer()->d2d1Factory();
        THROW_IF_FAILED(factory->CreatePathGeometry(&pathGeo));

        ComPtr<ID2D1GeometrySink> geoSink = {};
        THROW_IF_FAILED(pathGeo->Open(&geoSink));
        {
            auto triangle = sldr->valueLabelSideTriangleInShadow();

            geoSink->BeginFigure(triangle[0], D2D1_FIGURE_BEGIN_FILLED);

            geoSink->AddLines(triangle.data(), (UINT32)triangle.size());

            geoSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        }
        THROW_IF_FAILED(geoSink->Close());
    }

    const SharedPtr<Label>& Slider::valueLabel() const
    {
        return m_valueLabel;
    }

    void Slider::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        ///////////////////
        // Handle Shadow //
        ///////////////////
        {
            auto& shadow = handleRes.shadow;

            shadow.beginDraw(rndr->d2d1DeviceContext());
            {
                auto& setting = appearance().handle;

                resource_utils::solidColorBrush()->SetOpacity(1.0f);

                auto rect = math_utils::rect
                (
                    { 0.0f, 0.0f }, setting.geometry.size
                );
                D2D1_ROUNDED_RECT roundedRect =
                {
                    math_utils::moveVertex(rect, setting.shadow.offset),
                    setting.geometry.roundRadius, setting.geometry.roundRadius
                };
                rndr->d2d1DeviceContext()->FillRoundedRectangle
                (
                /* roundedRect */ roundedRect,
                /* brush       */ resource_utils::solidColorBrush()
                );
            }
            shadow.endDraw(rndr->d2d1DeviceContext());
        }
        //////////////////////
        // Value Label Mask //
        //////////////////////

        if (m_valueLabel->isD2d1ObjectVisible())
        {
            auto& mask = valueLabelRes.mask();

            mask.beginDraw(rndr->d2d1DeviceContext());
            {
                auto& setting = appearance().valueLabel;

                //------------------------------------------------------------------
                // Main rect
                //------------------------------------------------------------------
                {
                    auto& background = setting.mainRect.background;

                    resource_utils::solidColorBrush()->SetColor(background.color);
                    resource_utils::solidColorBrush()->SetOpacity(background.opacity);

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
                }
                //------------------------------------------------------------------
                // Side Triangle
                //------------------------------------------------------------------
                {
                    auto& background = setting.sideTriangle.background;

                    resource_utils::solidColorBrush()->SetColor(background.color);
                    resource_utils::solidColorBrush()->SetOpacity(background.opacity);

                    rndr->d2d1DeviceContext()->FillGeometry
                    (
                    /* geometry */ sideTriangleRes.pathGeo.Get(),
                    /* brush    */ resource_utils::solidColorBrush()
                    );
                }
            }
            mask.endDraw(rndr->d2d1DeviceContext());
        }
    }

    void Slider::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        //////////////////
        // Complete Bar //
        //////////////////
        {
            auto& setting = appearance().bar.complete;
            auto& background = m_enabled ? setting.background : setting.secondaryBackground;

            resource_utils::solidColorBrush()->SetColor(background.color);
            resource_utils::solidColorBrush()->SetOpacity(background.opacity);

            D2D1_ROUNDED_RECT roundedRect =
            {
                completeBarAbsoluteRect(),
                setting.geometry.roundRadius,
                setting.geometry.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ roundedRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        ////////////////
        // Filled Bar //
        ////////////////
        {
            auto& setting = appearance().bar.filled;
            auto& background = m_enabled ? setting.background : setting.secondaryBackground;

            resource_utils::solidColorBrush()->SetColor(background.color);
            resource_utils::solidColorBrush()->SetOpacity(background.opacity);

            D2D1_ROUNDED_RECT roundedRect =
            {
                filledBarAbsoluteRect(),
                setting.geometry.roundRadius,
                setting.geometry.roundRadius
            };
            rndr->d2d1DeviceContext()->FillRoundedRectangle
            (
            /* roundedRect */ roundedRect,
            /* brush       */ resource_utils::solidColorBrush()
            );
        }
        ////////////
        // Handle //
        ////////////
        {
            //------------------------------------------------------------------
            // Shadow
            //------------------------------------------------------------------
            {
                auto& shadow = handleRes.shadow;
                auto& setting = appearance().handle.shadow;

                shadow.color = m_enabled ? setting.color : setting.secondaryColor;

                shadow.configEffectInput(resource_utils::shadowEffect());

                rndr->d2d1DeviceContext()->DrawImage
                (
                /* effect       */ resource_utils::shadowEffect(),
                /* targetOffset */ math_utils::leftTop(handleAbsoluteRect())
                );
            }
            //------------------------------------------------------------------
            // Entity
            //------------------------------------------------------------------
            {
                auto& setting = appearance().handle;
                auto& background = m_enabled ? setting.background : setting.secondaryBackground;

                resource_utils::solidColorBrush()->SetColor(background.color);
                resource_utils::solidColorBrush()->SetOpacity(background.opacity);

                D2D1_ROUNDED_RECT roundedRect =
                {
                    handleAbsoluteRect(),
                    setting.geometry.roundRadius,
                    setting.geometry.roundRadius
                };
                rndr->d2d1DeviceContext()->FillRoundedRectangle
                (
                /* roundedRect */ roundedRect,
                /* brush       */ resource_utils::solidColorBrush()
                );
            }
        }
        /////////////////
        // Value Label //
        /////////////////

        if (appearance().valueLabel.resident)
        {
            m_valueLabel->setPrivateVisible(true);
            m_valueLabel->setPrivateEnabled(true);
        }
        if (m_valueLabel->isD2d1ObjectVisible())
        {
            auto rect = selfCoordToAbsolute(valueLabelShadowSelfCoordRect());

            //------------------------------------------------------------------
            // Shadow
            //------------------------------------------------------------------
            {
                auto& shadow = valueLabelRes.shadow();
                auto& setting = appearance().valueLabel.shadow;

                shadow.color = setting.color;
                shadow.standardDeviation = setting.standardDeviation;

                shadow.configEffectInput(resource_utils::shadowEffect());

                rndr->d2d1DeviceContext()->DrawImage
                (
                /* effect       */ resource_utils::shadowEffect(),
                /* targetOffset */ math_utils::leftTop(rect)
                );
            }
            //------------------------------------------------------------------
            // Entity
            //------------------------------------------------------------------
            {
                auto& mask = valueLabelRes.mask();

                rndr->d2d1DeviceContext()->DrawBitmap
                (
                /* bitmap               */ mask.data.Get(),
                /* destinationRectangle */ rect,
                /* opacity              */ mask.opacity,
                /* interpolationMode    */ mask.getInterpolationMode()
                );
            }
            //------------------------------------------------------------------
            // Text
            //------------------------------------------------------------------
            {
                m_valueLabel->onRendererDrawD2d1Object(rndr);
            }
        }
    }

    bool Slider::isHitHelper(const Event::Point& p) const
    {
        return math_utils::isOverlapped(p, thumbAreaRect(m_absoluteRect));
    }

    void Slider::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        /////////////////////////////
        // Reload Cached Resources //
        /////////////////////////////

        handleRes.loadShadow();

        sideTriangleRes.loadPathGeo();

        /////////////////////////////
        // Update Children Objects //
        /////////////////////////////

        m_valueLabel->transform(valueLabelSelfCoordRect());
    }

    void Slider::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Panel::onChangeThemeStyleHelper(style);

        appearance().changeTheme(style.name);
    }

    void Slider::onValueChangeHelper(float value)
    {
        ValuefulObject::onValueChangeHelper(value);

        /////////////////////////////
        // Update Children Objects //
        /////////////////////////////

        m_valueLabel->transform(valueLabelSelfCoordRect());

        std::wstringstream ss;

        auto& precision = appearance().valueLabel.precision;
        ss << std::fixed << std::setprecision(precision) << value;

        m_valueLabel->setText(ss.str());
    }

    void Slider::onStartSlidingHelper(float value)
    {
        SliderBase::onStartSlidingHelper(value);

        m_valueLabel->setPrivateVisible(true);
        m_valueLabel->setPrivateEnabled(true);
    }

    void Slider::onEndSlidingHelper(float value)
    {
        SliderBase::onEndSlidingHelper(value);

        m_valueLabel->setPrivateVisible(false);
        m_valueLabel->setPrivateEnabled(false);
    }
}
