﻿#include "Common/Precompile.h"

#include "UIKit/ViewItem.h"

#include "Common/CppLangUtils/PointerEquality.h"
#include "Common/MathUtils/2D.h"

#include "Renderer/Renderer.h"

#include "UIKit/IconLabel.h"
#include "UIKit/ResourceUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    ViewItem::ViewItem(
        ShrdPtrParam<Panel> content,
        const D2D1_RECT_F& rect)
        :
        Panel(rect, resource_utils::solidColorBrush()),
        contentMask(size()),
        m_content(content)
    {
        m_takeOverChildrenDrawing = true;
    }

    ViewItem::ViewItem(WstrParam text, const D2D1_RECT_F& rect)
        : ViewItem(IconLabel::uniformLayout(text), rect) { }


    void ViewItem::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        addUIObject(m_content);

        if (m_content) m_content->transform(selfCoordRect());
    }

    void ViewItem::setEnabled(bool value)
    {
        Panel::setEnabled(value);

        if (m_content) m_content->setEnabled(value);
    }

    WeakPtr<Panel> ViewItem::content() const
    {
        return m_content;
    }

    void ViewItem::setContent(ShrdPtrParam<Panel> content)
    {
        if (!cpp_lang_utils::isMostDerivedEqual(content, m_content))
        {
            removeUIObject(m_content);

            m_content = content;
            addUIObject(m_content);

            if (m_content) m_content->transform(selfCoordRect());
        }
    }

#define TARGET_STATE(Name) ViewItem::State::Name

    const ViewItem::StateTransitionMap
    ViewItem::ENTER_STATE_TRANS_MAP =
    {
    /* Idle               */ TARGET_STATE(Hover),
    /* Hover              */ TARGET_STATE(Hover),
    /* Selected           */ TARGET_STATE(Selected),
    /* FocusSelected      */ TARGET_STATE(FocusSelectedHover),
    /* FocusSelectedHover */ TARGET_STATE(FocusSelectedHover)
    },
    ViewItem::LEAVE_STATE_TRANS_MAP =
    {
    /* Idle               */ TARGET_STATE(Idle),
    /* Hover              */ TARGET_STATE(Idle),
    /* Selected           */ TARGET_STATE(Selected),
    /* FocusSelected      */ TARGET_STATE(FocusSelected),
    /* FocusSelectedHover */ TARGET_STATE(FocusSelected)
    },
    ViewItem::CHECK_STATE_TRANS_MAP =
    {
    /* Idle               */ TARGET_STATE(FocusSelectedHover),
    /* Hover              */ TARGET_STATE(FocusSelectedHover),
    /* Selected           */ TARGET_STATE(FocusSelectedHover),
    /* FocusSelected      */ TARGET_STATE(FocusSelectedHover),
    /* FocusSelectedHover */ TARGET_STATE(FocusSelectedHover)
    },
    ViewItem::UNCHK_STATE_TRANS_MAP =
    {
    /* Idle               */ TARGET_STATE(Idle),
    /* Hover              */ TARGET_STATE(Idle),
    /* Selected           */ TARGET_STATE(Idle),
    /* FocusSelected      */ TARGET_STATE(Idle),
    /* FocusSelectedHover */ TARGET_STATE(Idle)
    },
    ViewItem::GETFC_STATE_TRANS_MAP =
    {
    /* Idle               */ TARGET_STATE(Idle),
    /* Hover              */ TARGET_STATE(Hover),
    /* Selected           */ TARGET_STATE(FocusSelected),
    /* FocusSelected      */ TARGET_STATE(FocusSelected),
    /* FocusSelectedHover */ TARGET_STATE(FocusSelectedHover)
    },
    ViewItem::LOSFC_STATE_TRANS_MAP =
    {
    /* Idle               */ TARGET_STATE(Idle),
    /* Hover              */ TARGET_STATE(Hover),
    /* Selected           */ TARGET_STATE(Selected),
    /* FocusSelected      */ TARGET_STATE(Selected),
    /* FocusSelectedHover */ TARGET_STATE(Selected)
    };

#undef TARGET_STATE

    void ViewItem::triggerEnterStateTrans()
    {
        state = ENTER_STATE_TRANS_MAP[(size_t)state];
    }

    void ViewItem::triggerLeaveStateTrans()
    {
        state = LEAVE_STATE_TRANS_MAP[(size_t)state];
    }

    void ViewItem::triggerCheckStateTrans()
    {
        state = CHECK_STATE_TRANS_MAP[(size_t)state];
    }

    void ViewItem::triggerUnchkStateTrans()
    {
        state = UNCHK_STATE_TRANS_MAP[(size_t)state];
    }

    void ViewItem::triggerGetfcStateTrans()
    {
        state = GETFC_STATE_TRANS_MAP[(size_t)state];
    }

    void ViewItem::triggerLosfcStateTrans()
    {
        state = LOSFC_STATE_TRANS_MAP[(size_t)state];
    }

    void ViewItem::onRendererDrawD2d1LayerHelper(Renderer* rndr)
    {
        if (m_content && m_content->isD2d1ObjectVisible())
        {
            m_content->onRendererDrawD2d1Layer(rndr);

            auto& setting = getAppearance().main[(size_t)(m_enabled ? state : State::Idle)];

            //--------------------------------------------------------------
            // 1. Grayscale text anti-aliasing:
            // The rendering result is independent of the target background,
            // so opacity can be set as needed (any value from 0 ~ 1 is OK).
            //--------------------------------------------------------------
            // 2. ClearType text anti-aliasing:
            // The rendering result depends on the target background color,
            // so you must set an opaque background (better a value >= 0.5).
            //--------------------------------------------------------------
            contentMask.color = setting.background.color;
            contentMask.color.a = setting.background.opacity;

            auto maskDrawTrans = D2D1::Matrix3x2F::Translation
            (
                -m_absoluteRect.left, -m_absoluteRect.top
            );
            contentMask.beginDraw(rndr->d2d1DeviceContext(), maskDrawTrans);
            {
                m_content->onRendererDrawD2d1Object(rndr);
            }
            contentMask.endDraw(rndr->d2d1DeviceContext());
        }
    }

    void ViewItem::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        auto& setting = getAppearance().main[(size_t)(m_enabled ? state : State::Idle)];

        ////////////////
        // Background //
        ////////////////

        resource_utils::solidColorBrush()->SetColor(setting.background.color);
        resource_utils::solidColorBrush()->SetOpacity(setting.background.opacity);

        Panel::drawBackground(rndr);

        /////////////
        // Content //
        /////////////

        if (m_content && m_content->isD2d1ObjectVisible())
        {
            rndr->d2d1DeviceContext()->DrawBitmap
            (
            /* bitmap               */ contentMask.data.Get(),
            /* destinationRectangle */ m_absoluteRect,
            /* opacity              */ contentMask.opacity,
            /* interpolationMode    */ contentMask.getInterpolationMode()
            );
        }
        /////////////
        // Outline //
        /////////////

        resource_utils::solidColorBrush()->SetColor(setting.stroke.color);
        resource_utils::solidColorBrush()->SetOpacity(setting.stroke.opacity);

        auto frame = math_utils::inner(m_absoluteRect, setting.stroke.width);
        D2D1_ROUNDED_RECT outlineRect = { frame, roundRadiusX, roundRadiusY };

        rndr->d2d1DeviceContext()->DrawRoundedRectangle
        (
        /* roundedRect */ outlineRect,
        /* brush       */ resource_utils::solidColorBrush(),
        /* strokeWidth */ setting.stroke.width
        );
    }

    bool ViewItem::isHitHelper(const Event::Point& p) const
    {
        // Generally, two adjacent view-items adjoin each other in vertical
        // direction, and WaterfallView judges that the curosr-point hits the
        // second one if it is right on the border of the view-items, so we
        // need to exclude the bottom-edge of the view-item from the hit-test.

        return math_utils::isOverlappedExcludingBottom(p, m_absoluteRect);
    }

    bool ViewItem::releaseUIObjectHelper(ShrdPtrParam<Panel> uiobj)
    {
        if (cpp_lang_utils::isMostDerivedEqual(m_content, uiobj)) m_content.reset();

        return Panel::releaseUIObjectHelper(uiobj);
    }

    void ViewItem::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        contentMask.loadBitmap(e.size);

        if (m_content) m_content->transform(selfCoordRect());
    }

    void ViewItem::onChangeThemeStyleHelper(const ThemeStyle& style)
    {
        Panel::onChangeThemeStyleHelper(style);

        getAppearance().changeTheme(style.name);
    }
}
