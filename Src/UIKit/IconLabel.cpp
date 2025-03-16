#include "Common/Precompile.h"

#include "UIKit/IconLabel.h"

#include "Common/CppLangUtils/PointerEquality.h"
#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"

#include "Renderer/Renderer.h"

#include "UIKit/Label.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    IconLabel::IconLabel(
        WstrRefer labelText,
        BmpObjParam iconBitmap,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush())
    {
        m_takeOverChildrenDrawing = true;

        icon.rect = selfCoordRect();
        icon.bitmap = iconBitmap;

        m_label = makeUIObject<Label>(labelText);
    }

    void IconLabel::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        addUIObject(m_label);

        m_label->transform(selfCoordRect());
    }

    void IconLabel::setEnabled(bool value)
    {
        Panel::setEnabled(value);

        m_label->setEnabled(value);
    }

    const SharedPtr<Label>& IconLabel::label() const
    {
        return m_label;
    }

    void IconLabel::setLabel(ShrdPtrRefer<Label> label)
    {
        if (label && !cpp_lang_utils::isMostDerivedEqual(label, m_label))
        {
            removeUIObject(m_label);

            m_label = label;
            addUIObject(m_label);

            m_label->transform(selfCoordRect());
        }
    }

    void IconLabel::updateLayout()
    {
        if (f_updateLayout)
        {
            f_updateLayout(this);
        }
        else updateLayoutHelper();
    }

    void IconLabel::updateLayoutHelper()
    {
        // This method intentionally left blank.
    }

    SharedPtr<IconLabel> IconLabel::uniformLayout(
        WstrRefer labelText,
        BmpObjParam iconBitmap,
        const D2D1_RECT_F& rect)
    {
        auto iconLabel = makeUIObject<IconLabel>(labelText, iconBitmap, rect);

        THROW_IF_FAILED(iconLabel->m_label->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

        iconLabel->f_updateLayout = [](IconLabel* pIconLabel)
        {
            if (pIconLabel->icon.bitmap.data)
            {
                D2D1_SIZE_F iconSize = {};
                if (pIconLabel->icon.customSize.has_value())
                {
                    iconSize = pIconLabel->icon.customSize.value();
                }
                else // use the original size of the bitmap
                {
                    iconSize = pIconLabel->icon.bitmap.data->GetSize();
                }
                float textWidth = pIconLabel->m_label->textMetrics().widthIncludingTrailingWhitespace;

                float spacing = pIconLabel->width() - iconSize.width - textWidth;
                float padding = std::max(spacing / 3.0f, 0.0f); // even interval

                float iconRectRight = std::min(padding * 2.0f + iconSize.width, pIconLabel->width());
                float labelRectLeft = std::min(padding + iconSize.width, pIconLabel->width());

                pIconLabel->icon.rect = math_utils::centered(
                {
                    0.0f,
                    0.0f,
                    iconRectRight,
                    pIconLabel->height()
                },
                iconSize);

                pIconLabel->m_label->transform(
                {
                    labelRectLeft,
                    0.0f,
                    pIconLabel->width(),
                    pIconLabel->height()
                });
            }
            else // Display the text in the center.
            {
                pIconLabel->icon.rect = {};
                pIconLabel->m_label->transform(pIconLabel->selfCoordRect());
            }
        };
        // Perform the callback immediately to initialize the layout.
        iconLabel->f_updateLayout(iconLabel.get());

        return iconLabel;
    }

    SharedPtr<IconLabel> IconLabel::compactLayout(
        WstrRefer labelText,
        BmpObjParam iconBitmap,
        float iconHeadPadding,
        float iconTailPadding,
        const D2D1_RECT_F& rect)
    {
        auto iconLabel = makeUIObject<IconLabel>(labelText, iconBitmap, rect);

        iconLabel->f_updateLayout = [iconHeadPadding, iconTailPadding](IconLabel* pIconLabel)
        {
            D2D1_SIZE_F iconSize = { 0.0f, 0.0f };
            if (pIconLabel->icon.customSize.has_value())
            {
                iconSize = pIconLabel->icon.customSize.value();
            }
            else if (pIconLabel->icon.bitmap.data != nullptr)
            {
                iconSize = pIconLabel->icon.bitmap.data->GetSize();
            }
            float iconRectRight = std::min(iconHeadPadding + iconSize.width, pIconLabel->width());
            float labelRectLeft = std::min(iconRectRight + iconTailPadding, pIconLabel->width());

            pIconLabel->icon.rect = math_utils::centered(
            {
                iconHeadPadding,
                0.0f,
                iconRectRight,
                pIconLabel->height()
            },
            iconSize);

            pIconLabel->m_label->transform(
            {
                labelRectLeft,
                0.0f,
                pIconLabel->width(),
                pIconLabel->height()
            });
        };
        // Perform the callback immediately to initialize the layout.
        iconLabel->f_updateLayout(iconLabel.get());

        return iconLabel;
    }

    SharedPtr<IconLabel> IconLabel::iconExpandedLayout(
        WstrRefer labelText,
        BmpObjParam iconBitmap,
        const D2D1_RECT_F& rect)
    {
        auto iconLabel = makeUIObject<IconLabel>(labelText, iconBitmap, rect);

        iconLabel->f_updateLayout = [](IconLabel* pIconLabel)
        {
            D2D1_SIZE_F iconSize = { 0.0f, 0.0f };
            if (pIconLabel->icon.customSize.has_value())
            {
                iconSize = pIconLabel->icon.customSize.value();
            }
            else if (pIconLabel->icon.bitmap.data != nullptr)
            {
                iconSize = pIconLabel->icon.bitmap.data->GetSize();
            }
            float textWidth = pIconLabel->m_label->textMetrics().width;
            float boundaryOffset = pIconLabel->width() - std::min(textWidth, pIconLabel->width());

            pIconLabel->icon.rect = math_utils::centered(
            {
                0.0f,
                0.0f,
                boundaryOffset,
                pIconLabel->height()
            },
            iconSize);

            pIconLabel->m_label->transform(
            {
                boundaryOffset,
                0.0f,
                pIconLabel->width(),
                pIconLabel->height()
            });
        };
        // Perform the callback immediately to initialize the layout.
        iconLabel->f_updateLayout(iconLabel.get());

        return iconLabel;
    }

    SharedPtr<IconLabel> IconLabel::labelExpandedLayout(
        WstrRefer labelText,
        BmpObjParam iconBitmap,
        const D2D1_RECT_F& rect)
    {
        auto iconLabel = makeUIObject<IconLabel>(labelText, iconBitmap, rect);

        THROW_IF_FAILED(iconLabel->m_label->textLayout()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));

        iconLabel->f_updateLayout = [](IconLabel* pIconLabel)
        {
            D2D1_SIZE_F iconSize = { 0.0f, 0.0f };
            if (pIconLabel->icon.customSize.has_value())
            {
                iconSize = pIconLabel->icon.customSize.value();
            }
            else if (pIconLabel->icon.bitmap.data != nullptr)
            {
                iconSize = pIconLabel->icon.bitmap.data->GetSize();
            }
            float boundaryOffset = std::min(iconSize.width, pIconLabel->width());

            pIconLabel->icon.rect = math_utils::centered(
            {
                0.0f,
                0.0f,
                boundaryOffset,
                pIconLabel->height()
            },
            iconSize);

            pIconLabel->m_label->transform(
            {
                boundaryOffset,
                0.0f,
                pIconLabel->width(),
                pIconLabel->height()
            });
        };
        // Perform the callback immediately to initialize the layout.
        iconLabel->f_updateLayout(iconLabel.get());

        return iconLabel;
    }

    SharedPtr<IconLabel> IconLabel::comboBoxLayout(
        WstrRefer labelText,
        BmpObjParam iconBitmap,
        const D2D1_RECT_F& rect)
    {
        return compactLayout(labelText, iconBitmap, 12.0f, 0.0f, rect);
    }

    void IconLabel::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        updateLayout();
    }

    bool IconLabel::releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj)
    {
        if (cpp_lang_utils::isMostDerivedEqual(uiobj, m_label)) return false;

        return Panel::releaseUIObjectHelper(uiobj);
    }

    void IconLabel::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        ////////////////
        // Label Text //
        ////////////////

        if (m_label->isD2d1ObjectVisible())
        {
            m_label->onRendererDrawD2d1Object(rndr);
        }

        /////////////////
        // Icon Bitmap //
        /////////////////

        if (icon.bitmap.data)
        {
            rndr->d2d1DeviceContext()->DrawBitmap
            (
            /* bitmap               */ icon.bitmap.data.Get(),
            /* destinationRectangle */ selfCoordToAbsolute(icon.rect),
            /* opacity              */ icon.bitmap.opacity,
            /* interpolationMode    */ icon.bitmap.getInterpolationMode()
            );
        }
    }
}
