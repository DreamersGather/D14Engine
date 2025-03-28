﻿#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/ComboBox.h"
#include "UIKit/FlatButton.h"

namespace d14engine::uikit
{
    struct IconLabel;
    struct MenuItem;
    struct PopupMenu;

    struct ComboBox : appearance::ComboBox, FlatButton
    {
        // The typical height is 40, i.e. math_utils::height(rect) == 40.
        ComboBox(float roundRadius = 0.0f, const D2D1_RECT_F& rect = {});

        void onInitializeFinish() override;

        struct ArrowIcon
        {
            ComPtr<ID2D1StrokeStyle> strokeStyle = {};
        }
        arrowIcon = {};

        void loadArrowIconStrokeStyle();

        _D14_SET_APPEARANCE_PROPERTY(ComboBox)

    public:
        void onSelectedChange(IconLabel* content);

        Function<void(ComboBox*, IconLabel*)> f_onSelectedChange = {};

    protected:
        void onSelectedChangeHelper(IconLabel* content);

    protected:
        WeakPtr<MenuItem> m_selected = {};

        SharedPtr<PopupMenu> m_dropDownMenu = {};

    public:
        const WeakPtr<MenuItem>& selected() const;
        void setSelected(size_t indexInDropDownMenu);

        const SharedPtr<PopupMenu>& dropDownMenu() const;
        void setDropDownMenu(ShrdPtrRefer<PopupMenu> menu);

    public:
        Optional<D2D1_POINT_2F> menuOffset = {};

    protected:
        // IDrawObject2D
        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        // Panel
        void onSizeHelper(SizeEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        // ClickablePanel
        void onMouseButtonReleaseHelper(ClickablePanel::Event& e) override;
    };
}
