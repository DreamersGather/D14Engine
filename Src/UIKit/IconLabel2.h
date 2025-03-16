#pragma once

#include "Common/Precompile.h"

#include "IconLabel.h"

namespace d14engine::uikit
{
    struct IconLabel2 : IconLabel
    {
        IconLabel2(
            WstrRefer labelText = {},
            WstrRefer label2Text = {},
            BmpObjParam iconBitmap = {},
            const D2D1_RECT_F& rect = {});

        void onInitializeFinish() override;

        void setEnabled(bool value) override;

    protected:
        SharedPtr<Label> m_label2 = {};

    public:
        const SharedPtr<Label>& label2() const;
        void setLabel2(ShrdPtrRefer<Label> label);

    public:
        static SharedPtr<IconLabel2> menuItemLayout(
            WstrRefer labelText = {},
            WstrRefer hotkeyText = {},
            BmpObjParam iconBitmap = {},
            float textHeadPadding = 30.0f,
            float hotkeyTailPadding = 30.0f,
            const D2D1_RECT_F& rect = {});

    protected:
        bool releaseUIObjectHelper(ShrdPtrRefer<Panel> uiobj) override;
        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;
    };
}
