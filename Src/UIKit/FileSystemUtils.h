#pragma once

#include "Common/Precompile.h"

namespace d14engine::file_system_utils
{
    // Example 1: path = L"asdf.cpp", return = L"asdf.cpp".
    // Example 2: path = L"dir/file.cpp", return = L"file.cpp".
    // Example 4: path = L"dir\\file.cpp", return = L"file.cpp".
    // Example 3: path = L"first/second/", return = L"".
    Wstring extractFileName(WstrParam path);

    // Example 1: fileName = L"asdf.cpp", return = L"asdf".
    // Example 2: fileName = L"dir/file.cpp", return = L"dir/file".
    // Example 3: fileName = L"dir\\file.cpp", return = L"dir\\file".
    // Example 4: fileName = L"first/second/", return = L"first/second".
    Wstring extractFilePrefix(WstrParam fileName, WCHAR token = L'.');

    // Example 1: fileName = L"asdf.cpp", return = L"cpp".
    // Example 2: fileName = L"dir/file.cpp", return = L"cpp".
    // Example 3: fileName = L"dir\\file.cpp", return = L"cpp".
    // Example 4: fileName = L"first/second/", return = L"".
    Wstring extractFileSuffix(WstrParam fileName, WCHAR token = L'.');

    // Return true to terminate searching next file in foreachFileInDir.
    using FileOperationFunc = Function<bool(WstrParam /* file path */)>;

    // Returns whether the specific files are located successfully.
    // wildcard examples: "*" for all, "*.png" for PNG images, etc.
    bool foreachFileInDir(WstrParam dir, WstrParam wildcard, const FileOperationFunc& func);
}
