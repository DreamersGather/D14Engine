﻿#pragma once

#include "Common/Precompile.h"

#include "Common/CppLangUtils/TypeTraits.h"

namespace d14engine::cpp_lang_utils
{
    // This struct helps pass index and iterator between different routines,
    // and the design concept is that we use the index to check the validity
    // and use the iterator to fetch the entity.

    template<typename T>
    struct IndexIterator
    {
        static_assert(isInstanceOf<T, std::list>,
            "T must be an instance of std::list");

        using Type = IndexIterator<T>;

    private:
        T* m_pList = nullptr;

    public:
        size_t index = {};

        typename T::iterator iterator = {};

        IndexIterator(T* pList = nullptr)
            :
            IndexIterator(pList, 0) { }

        IndexIterator(T* pList, size_t index)
            :
            m_pList(pList)
        {
            if (m_pList != nullptr)
            {
                move(index);
            }
            else invalidate();
        }

        IndexIterator(const Type& rhs)
            :
            m_pList(rhs.m_pList),
            index(rhs.index),
            iterator(rhs.iterator) { }

        Type& operator=(const Type& rhs)
        {
            if (this == &rhs)
            {
                return *this;
            }
            m_pList = rhs.m_pList;
            index = rhs.index;
            iterator = rhs.iterator;

            return *this;
        }

        Type& operator++()
        {
            ++index;
            ++iterator;
            return *this;
        }

        Type operator++(int)
        {
            Type old = *this;
            operator++();
            return old;
        }

        Type& operator--()
        {
            --index;
            --iterator;
            return *this;
        }

        Type operator--(int)
        {
            Type old = *this;
            operator--();
            return old;
        }

        operator size_t() const
        {
            return index;
        }

        auto operator<=>(size_t rhs) const
        {
            return index <=> rhs;
        }

        bool operator==(size_t other) const
        {
            return index == other;
        }

        auto operator<=>(const Type& rhs) const
        {
            return index <=> rhs.index;
        }

        bool operator==(const Type& other) const
        {
            return index == other.index;
        }

        typename T::value_type& operator*() const
        {
            return *iterator;
        }

        typename T::value_type* operator->() const
        {
            return &(*iterator);
        }

        static IndexIterator begin(T* pList)
        {
            IndexIterator tmp{ pList };
            tmp.moveBegin();
            return tmp;
        }

        static IndexIterator end(T* pList)
        {
            IndexIterator tmp{ pList };
            tmp.moveEnd();
            return tmp;
        }

        static IndexIterator last(T* pList)
        {
            IndexIterator tmp{ pList };
            tmp.moveLast();
            return tmp;
        }

        bool valid() const
        {
            return m_pList != nullptr && index < m_pList->size();
        }

        // general-valid only means that invalidate is not called,
        // but the iterator may be invalid if it has been changed!
        bool generalValid() const
        {
            return m_pList != nullptr && index != SIZE_MAX;
        }

        Type& invalidate()
        {
            m_pList = nullptr;
            index = SIZE_MAX;
            iterator = typename T::iterator{};
            return *this;
        }

        bool atBegin() const
        {
            return valid() && index == 0;
        }

        bool atEnd() const
        {
            return m_pList != nullptr && index == m_pList->size();
        }

        bool atLast() const
        {
            return valid() && index == m_pList->size() - 1;
        }

        Type& move(size_t index)
        {
            this->index = index;
            iterator = std::next(m_pList->begin(), index);
            return *this;
        }

        Type& moveBegin()
        {
            index = 0;
            iterator = m_pList->begin();
            return *this;
        }

        Type& moveEnd()
        {
            index = m_pList->size();
            iterator = m_pList->end();
            return *this;
        }

        Type& moveLast()
        {
            index = m_pList->size() - 1;
            iterator = --m_pList->end();
            return *this;
        }

        Type& movePrev(size_t offset = 1)
        {
            index -= offset;
            iterator = std::prev(iterator, offset);
            return *this;
        }

        Type& moveIndexPrev(size_t offset = 1)
        {
            index -= offset;
            return *this;
        }

        Type& moveItorPrev(size_t offset = 1)
        {
            iterator = std::prev(iterator, offset);
            return *this;
        }

        Type getPrev(size_t offset = 1) const
        {
            Type tmp = *this;
            tmp.index -= offset;
            tmp.iterator = std::prev(iterator, offset);
            return tmp;
        }

        Type getIndexPrev(size_t offset = 1) const
        {
            Type tmp = *this;
            tmp.index -= offset;
            return tmp;
        }

        Type getItorPrev(size_t offset = 1) const
        {
            Type tmp = *this;
            tmp.iterator = std::prev(iterator, offset);
            return tmp;
        }

        Type& moveNext(size_t offset = 1)
        {
            index += offset;
            iterator = std::next(iterator, offset);
            return *this;
        }

        Type& moveIndexNext(size_t offset = 1)
        {
            index += offset;
            return *this;
        }

        Type& moveItorNext(size_t offset = 1)
        {
            iterator = std::next(iterator, offset);
            return *this;
        }

        Type getNext(size_t offset = 1) const
        {
            Type tmp = *this;
            tmp.index += offset;
            tmp.iterator = std::next(iterator, offset);
            return tmp;
        }

        Type getIndexNext(size_t offset = 1) const
        {
            Type tmp = *this;
            tmp.index += offset;
            return tmp;
        }

        Type getItorNext(size_t offset = 1) const
        {
            Type tmp = *this;
            tmp.iterator = std::next(iterator, offset);
            return tmp;
        }
    };
}
