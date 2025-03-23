#pragma once

#include "Common/Precompile.h"

namespace d14engine::cpp_lang_utils
{
    // We can get the address of the most-derived object by casting void-ptr,
    // which is very helpful for the comparison in multiple inheritance.
    //
    // Besides, it is recommended to use dynamic_cast to be compatible with
    // the potential virtual inheritance at the tolerable cost of speed.

    template<typename T, typename U>
    bool isMostDerivedEqual(T* lhs, U* rhs)
    {
        using VoidPtr = const void*;
        return dynamic_cast<VoidPtr>(lhs) == dynamic_cast<VoidPtr>(rhs);
    }

    template<typename T, typename U>
    bool isMostDerivedEqual(ShrdPtrRefer<T> lhs, ShrdPtrRefer<U> rhs)
    {
        return isMostDerivedEqual(lhs.get(), rhs.get());
    }

    template<typename T, typename U>
    bool isMostDerivedEqual(WeakPtrRefer<T> lhs, WeakPtrRefer<U> rhs)
    {
        return isMostDerivedEqual(lhs.lock(), rhs.lock());
    }
}
