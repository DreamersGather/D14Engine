#pragma once

#include "Common/Precompile.h"

#include "UIKit/ConstraintLayout.h"
#include "UIKit/TreeViewItem.h"

d14engine::SharedPtr<d14engine::uikit::TreeViewItem> createLayoutPage
(std::unordered_map<d14engine::Wstring, d14engine::SharedPtr<d14engine::uikit::ConstraintLayout>>& pages);
