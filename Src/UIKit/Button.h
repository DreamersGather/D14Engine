#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/Button.h"
#include "UIKit/ClickablePanel.h"

namespace d14engine::uikit
{
    struct IconLabel;

    struct Button : appearance::Button, ClickablePanel
    {
        Button(
            ShrdPtrRefer<IconLabel> content,
            float roundRadius = 0.0f,
            const D2D1_RECT_F& rect = {});

        Button(
            WstrRefer text = L"Button",
            float roundRadius = 0.0f,
            const D2D1_RECT_F& rect = {});

        void onInitializeFinish() override;

        _D14_SET_APPEARANCE_PROPERTY(Button)

        /////////////////////////
        // Graphics Components //
        /////////////////////////

        //------------------------------------------------------------------
        // Children Objects
        //------------------------------------------------------------------
    protected:
        SharedPtr<IconLabel> m_content = {};

    public:
        const SharedPtr<IconLabel>& content() const;
        void setContent(ShrdPtrRefer<IconLabel> content);

        ///////////////////////
        // Interaction Logic //
        ///////////////////////

        //------------------------------------------------------------------
        // State
        //------------------------------------------------------------------
    public:
        enum class State
        {
            Idle,
            Hover,
            Down,
            Disabled
        };

    protected:
        State m_state = State::Idle;

    public:
        State state() const;

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

        bool releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj) override;

        bool isHitHelper(const uikit::Event::Point& p) const override;

        void onSizeHelper(SizeEvent& e) override;

        void onMouseEnterHelper(MouseMoveEvent& e) override;

        void onMouseLeaveHelper(MouseMoveEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;

        //------------------------------------------------------------------
        // ClickablePanel
        //------------------------------------------------------------------

        void onMouseButtonPressHelper(Event& e) override;

        void onMouseButtonReleaseHelper(Event& e) override;
    };
}
