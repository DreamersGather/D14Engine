#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/TypeTraits.h"

namespace d14engine::uikit
{
    template<typename State_T, typename StateDetail_T>
    struct StatefulObject
    {
        static_assert(cpp_lang_utils::has_operator::equalTo<StateDetail_T>,
            "An equal-to operator must be implemented for StateDetail_T");

        using State = State_T;
        using Event = StateDetail_T;

    public:
        void onStateChange(StateDetail_T& e)
        {
            onStateChangeHelper(e);

            if (f_onStateChange) f_onStateChange(this, e);
        }

        Function<void(StatefulObject*, StateDetail_T&)> f_onStateChange = {};

    protected:
        virtual void onStateChangeHelper(StateDetail_T& e) { }

    protected:
        State_T m_state = {};
        StateDetail_T m_stateDetail = {};

    public:
        const StateDetail_T& stateDetail() const { return m_stateDetail; }
    };
}
