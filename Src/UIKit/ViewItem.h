#pragma once

#include "Common/Precompile.h"

#include "UIKit/Appearances/ViewItem.h"
#include "UIKit/MaskObject.h"
#include "UIKit/Panel.h"

namespace d14engine::uikit
{
    template<typename>
    struct WaterfallView;

    struct ViewItem : appearance::ViewItem, Panel
    {
        // Due to the syntax issue of friend template,
        // the keyword (struct) here cannot be omitted.
        template<typename>
        friend struct WaterfallView;

        ViewItem(ShrdPtrParam<Panel> content, const D2D1_RECT_F& rect = {});

        ViewItem(WstrParam text = L"ViewItem", const D2D1_RECT_F& rect = {});

        void onInitializeFinish() override;

        _D14_SET_APPEARANCE_GETTER(ViewItem)

        //////////////////////
        // Cached Resources //
        //////////////////////

        using MasterPtr = cpp_lang_utils::EnableMasterPtr<ViewItem>;

        struct DrawBuffer : MasterPtr
        {
            using MasterPtr::MasterPtr;

            MaskObject mask = {};
            void loadMask();
        }
        drawBufferRes{ this };

        /////////////////////////
        // Graphics Components //
        /////////////////////////

        //------------------------------------------------------------------
        // Children Objects
        //------------------------------------------------------------------
    protected:
        // NO WeakPtr here as we provide a content-param in the ctor.
        SharedPtr<Panel> m_content = {};

    public:
        template<typename T = Panel>
        WeakPtr<T> getContent() const
        {
            if constexpr (std::is_same_v<T, Panel>) return content();
            else return std::dynamic_pointer_cast<T>(content().lock());
        }
        WeakPtr<Panel> content() const;
        void setContent(ShrdPtrParam<Panel> content);

        ///////////////////////
        // Interaction Logic //
        ///////////////////////

        //------------------------------------------------------------------
        // State & Transition Map
        //------------------------------------------------------------------
    public:
        State state = State::Idle;

        using StateTransitionMap = cpp_lang_utils::EnumMap<State>;

        const static StateTransitionMap
            ENTER_STATE_TRANS_MAP,
            LEAVE_STATE_TRANS_MAP,
            CHECK_STATE_TRANS_MAP,
            UNCHK_STATE_TRANS_MAP,
            GETFC_STATE_TRANS_MAP,
            LOSFC_STATE_TRANS_MAP;

        void triggerEnterStateTrans();
        void triggerLeaveStateTrans();
        void triggerCheckStateTrans();
        void triggerUnchkStateTrans();
        void triggerGetfcStateTrans();
        void triggerLosfcStateTrans();

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

        void onRendererDrawD2d1LayerHelper(renderer::Renderer* rndr) override;

        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        //------------------------------------------------------------------
        // Panel
        //------------------------------------------------------------------

        bool isHitHelper(const Event::Point& p) const override;

        bool releaseUIObjectHelper(ShrdPtrParam<Panel> uiobj) override;

        void onSizeHelper(SizeEvent& e) override;

        void onChangeThemeStyleHelper(const ThemeStyle& style) override;
    };
}
