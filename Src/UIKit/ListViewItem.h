#pragma once

#include "Common/Precompile.h"

#include "UIKit/ViewItem.h"

namespace d14engine::uikit
{
    struct ListView;

    struct ListViewItem : ViewItem
    {
        using ViewItem::ViewItem;

        friend ListView;

        const WeakPtr<ListView>& parentView() const
        {
            return m_parentView;
        }
    protected:
        WeakPtr<ListView> m_parentView = {};
    };
}
