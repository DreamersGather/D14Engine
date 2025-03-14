﻿#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/CheckBox.h"
#include "UIKit/ClickablePanel.h"
#include "UIKit/StatefulObject.h"

namespace d14engine::uikit
{
    struct CheckBox
        :
        appearance::CheckBox, ClickablePanel,
        StatefulObject<CheckBoxState, CheckBoxStateChangeEvent>
    {
        CheckBox(
            bool isTripleState = false,
            float roundRadius = 4.0f,
            const D2D1_RECT_F& rect = { 0.0f, 0.0f, 24.0f, 24.0f });

        void onInitializeFinish() override;

        struct CheckedIcon
        {
            ComPtr<ID2D1StrokeStyle> strokeStyle = {};
        }
        checkedIcon = {};

        void loadCheckedIconStrokeStyle();

        _D14_SET_APPEARANCE_PROPERTY(CheckBox)

        void setEnabled(bool value) override;

    public:
        constexpr static auto Unchecked = State::ActiveFlag::Unchecked;
        constexpr static auto Intermediate = State::ActiveFlag::Intermediate;
        constexpr static auto Checked = State::ActiveFlag::Checked;

        void setChecked(State::ActiveFlag flag);
        // Set the state directly without callback.
        void setCheckedState(State::ActiveFlag flag);

    protected:
        bool m_isTripleState = {};

        using StateTransitionMap = cpp_lang_utils::EnumMap<State::ActiveFlag>;

        StateTransitionMap m_stateTransitionMap = {};

    public:
        bool isTripleState() const;
        void enableTripleState(bool value);

    protected:
        // IDrawObject2D
        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        // Panel
        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        void onMouseEnterHelper(MouseMoveEvent& e) override;

        void onMouseLeaveHelper(MouseMoveEvent& e) override;

        // ClickablePanel
        void onMouseButtonPressHelper(ClickablePanel::Event& e) override;

        void onMouseButtonReleaseHelper(ClickablePanel::Event& e) override;
    };
}
