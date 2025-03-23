#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/EmptyBase.h"

// Do NOT remove this header for code tidy
// as the template deduction relies on it.
#include "Common/CppLangUtils/TypeTraits.h"

namespace d14engine
{
    // Define ISortable as a template class, so that an object can implement
    // sorting functionality for different properties by inheriting
    // multiple ISortable<Target_T> (where T represents different properties).
    //
    // For example, suppose struct Student is inherited from
    // ISortable<Height> and ISortable<Weight>, in which case a student knows
    // their height and weight respectively and can be sorted by each of them.

    template<typename Target_T, bool Inherit = false, typename Priority_T = int>
    struct ISortable : std::conditional_t<Inherit, Target_T, cpp_lang_utils::EmptyBase>
    {
        using Type = ISortable<Target_T, Inherit, Priority_T>;

#define SET_OPERATOR_REQUIRE(Operator) \
static_assert \
( \
    cpp_lang_utils::has_operator::Operator<Priority_T>, \
    "TOR requires a " #Operator " operator implementation for Priority_T" \
);
        SET_OPERATOR_REQUIRE(equalTo)
        SET_OPERATOR_REQUIRE(lessThan)

#undef SET_OPERATOR_REQUIRE

    protected:
        Priority_T m_priority = {};

    public:
        const Type* id() const { return this; }

        Priority_T priority() const { return m_priority; }
        void setPriority(Priority_T value) { m_priority = value; }

        ///////////////////////
        // Raw Ptr Ascending //
        ///////////////////////

        struct RawAscending
        {
            bool operator()(const Type& lhs, const Type& rhs) const
            {
                if (lhs.id() == rhs.id())
                {
                    return false;
                }
                else // Strict unique according to ID.
                {
                    if (lhs.priority() == rhs.priority())
                    {
                        return lhs.id() < rhs.id();
                    }
                    else // Strict ascending according to priority.
                    {
                        return lhs.priority() < rhs.priority();
                    }
                }
            }
        };
        using RawPrioritySet = std::set<Target_T, RawAscending>;

        template<typename ValueType>
        using RawPriorityMap = std::map<Target_T, ValueType, RawAscending>;

        //////////////////////////
        // Shared Ptr Ascending //
        //////////////////////////

        struct ShrdAscending
        {
            bool operator()(ShrdPtrRefer<Type> lhs, ShrdPtrRefer<Type> rhs) const
            {
                return RawAscending()(*lhs.get(), *rhs.get());
            }
        };
        using ShrdPrioritySet = std::set<SharedPtr<Target_T>, ShrdAscending>;

        template<typename ValueType>
        using ShrdPriorityMap = std::map<SharedPtr<Target_T>, ValueType, ShrdAscending>;

        ////////////////////////
        // Weak Ptr Ascending //
        ////////////////////////

        struct WeakAscending
        {
            bool operator()(WeakPtrRefer<Type> lhs, WeakPtrRefer<Type> rhs) const
            {
                // The order of comparison here is significant:
                //
                // 1. Compare "rhs" in advance of "lhs":
                //
                //    Place the expired elements in front of the set.
                //
                // 2. Return "false" in advance of "true":
                //
                //    Ensure the TOR (compatible with STL).
                //
                //-----------------------------------------------
                // TOR: Total Order Relation
                //-----------------------------------------------
                // 1. Reflexive:
                //    assert(a <= a)
                //-----------------------------------------------
                // 2. Transitive:
                //    If (a <= b && b <= c) then (a <= c)
                //-----------------------------------------------
                // 3. Antisymmetric:
                //    If (a <= b && b <= a) then (a == b)
                //-----------------------------------------------
                // 4. Strongly Connected (Total):
                //    assert(a <= b || b <= a)
                //-----------------------------------------------

                if (rhs.expired()) return false;
                if (lhs.expired()) return true;

                return ShrdAscending()(lhs.lock(), rhs.lock());
            }
        };
        using WeakPrioritySet = std::set<WeakPtr<Target_T>, WeakAscending>;

        template<typename ValueType>
        using WeakPriorityMap = std::map<WeakPtr<Target_T>, ValueType, WeakAscending>;

        ///////////////////
        // Miscellaneous //
        ///////////////////

        // The boolean value returned by func
        // indicates whether to handle the remainings.
        static void foreach(
            WeakPrioritySet& cont,
            FuncRefer<bool(ShrdPtrRefer<Target_T>)> func)
        {
            bool continueDeliver = true;

            for ( auto itor = cont.begin() ;; )
            {
                while (itor != cont.end() && itor->expired())
                {
                    itor = cont.erase(itor);
                }
                if (itor != cont.end())
                {
                    if (continueDeliver)
                    {
                        continueDeliver = func(itor->lock());
                    }
                    ++itor;
                }
                else break;
            }
        }
    };
}
