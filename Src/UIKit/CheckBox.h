#pragma once

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
        enum class CheckMode { Binary, TriState };

        constexpr static auto Binary = CheckMode::Binary;
        constexpr static auto TriState = CheckMode::TriState;

        CheckBox(
            CheckMode mode = Binary,
            float roundRadius = 4.0f,
            const D2D1_RECT_F& rect = { 0.0f, 0.0f, 24.0f, 24.0f });

        void onInitializeFinish() override;

        _D14_SET_APPEARANCE_PROPERTY(CheckBox)

        //////////////////////
        // Cached Resources //
        //////////////////////

        using MasterPtr = cpp_lang_utils::EnableMasterPtr<CheckBox>;

        struct CheckedIcon : MasterPtr
        {
            using MasterPtr::MasterPtr;

            ComPtr<ID2D1StrokeStyle> strokeStyle = {};
            void loadStrokeStyle();
        }
        checkedIcon{ this };

        ///////////////////////
        // Interaction Logic //
        ///////////////////////

        //------------------------------------------------------------------
        // Check Mode
        //------------------------------------------------------------------
    protected:
        CheckMode m_mode = {};

    public:
        CheckMode checkMode() const;
        void setCheckMode(CheckMode mode);

        //------------------------------------------------------------------
        // Check State
        //------------------------------------------------------------------
    public:
        using CheckState = State::ActiveFlag;

        constexpr static auto Unchecked = CheckState::Unchecked;
        constexpr static auto Intermediate = CheckState::Intermediate;
        constexpr static auto Checked = CheckState::Checked;

        CheckState checkState() const;
        void setCheckState(CheckState state);

        // Update the state without triggering the corresponding event.
        void setCheckStateSilently(CheckState state);

        //------------------------------------------------------------------
        // State Maps
        //------------------------------------------------------------------
    protected:
        using StateMap = cpp_lang_utils::EnumMap<CheckState>;
        using StateMapGroup = cpp_lang_utils::EnumMap<CheckMode, StateMap>;

        const static StateMapGroup g_stateMaps;

        const StateMap& stateMap() const; CheckState nextState() const;

        /////////////////////////
        // Interface Overrides //
        /////////////////////////

    public:
        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        void setEnabled(bool value) override;

    protected:
        //------------------------------------------------------------------
        // IDrawObject2D
        //------------------------------------------------------------------

        void onRendererDrawD2d1ObjectHelper(Renderer* rndr) override;

        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        void onMouseEnterHelper(MouseMoveEvent& e) override;

        void onMouseLeaveHelper(MouseMoveEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        //------------------------------------------------------------------
        // ClickablePanel
        //------------------------------------------------------------------

        void onMouseButtonPressHelper(ClickablePanel::Event& e) override;

        void onMouseButtonReleaseHelper(ClickablePanel::Event& e) override;
    };
}
