#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/TabCaption.h"
#include "UIKit/Panel.h"

namespace d14engine::uikit
{
    struct IconLabel;
    struct TabGroup;

    struct TabCaption : appearance::TabCaption, Panel
    {
        friend TabGroup;

        TabCaption(ShrdPtrRefer<IconLabel> title);

        explicit TabCaption(WstrRefer text = L"Untitled");

        void onInitializeFinish() override;

        _D14_SET_APPEARANCE_PROPERTY(TabCaption)

        void setEnabled(bool value) override;

    protected:
        SharedPtr<IconLabel> m_title = {};

        WeakPtr<TabGroup> m_parentTabGroup = {};

    public:
        const SharedPtr<IconLabel>& title() const;
        void setTitle(ShrdPtrRefer<IconLabel> title);

        const WeakPtr<TabGroup>& parentTabGroup() const;

    protected:
        D2D1_RECT_F titleSelfcoordRect() const;

        D2D1_POINT_2F closeButtonAbsolutePosition() const;

        D2D1_RECT_F closeIconAbsoluteRect() const;
        D2D1_RECT_F closeButtonAbsoluteRect() const;

    public:
        bool closable = true, draggable = true, promotable = false;

    protected:
        bool m_isCloseButtonHover = false;
        bool m_isCloseButtonDown = false;

        ButtonState getCloseButtonState() const;

    protected:
        // IDrawObject2D
        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        // Panel
        bool isHitHelper(const Event::Point& p) const override;

        void onSizeHelper(SizeEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        void onMouseMoveHelper(MouseMoveEvent& e) override;

        void onMouseLeaveHelper(MouseMoveEvent& e) override;

        void onMouseButtonHelper(MouseButtonEvent& e) override;
    };
}
