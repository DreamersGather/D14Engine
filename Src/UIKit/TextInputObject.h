#pragma once

#include "Common/Precompile.h"

namespace d14engine::uikit
{
    struct TextInputObject
    {
        virtual Optional<LOGFONT> getCompositionFont() const
        {
            return std::nullopt;
        }
        virtual Optional<COMPOSITIONFORM> getCompositionForm() const
        {
            return std::nullopt;
        }

        void OnInputString(WstrRefer str)
        {
            onInputStringHelper(str);

            if (f_onInputString) f_onInputString(this, str);
        }

        Function<void(TextInputObject*, WstrRefer)> f_onInputString = {};

        void onTextChange(WstrRefer text)
        {
            onTextChangeHelper(text);

            if (f_onTextChange) f_onTextChange(this, text);
        }

        Function<void(TextInputObject*, WstrRefer)> f_onTextChange = {};

    protected:
        virtual void onInputStringHelper(WstrRefer str) { }
        virtual void onTextChangeHelper(WstrRefer text) { }
    };
}
