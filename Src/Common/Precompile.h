﻿#pragma once

//////////////////////
// Standard Library //
//////////////////////

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iomanip>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace d14engine
{
    template<typename T>
    using Function = std::function<T>;
    template<typename T>
    using FuncParam = const Function<T>&;

    template<typename T>
    using Optional = std::optional<T>;
    template<typename T>
    using OptParam = const Optional<T>&;

    template<typename T>
    using SharedPtr = std::shared_ptr<T>;
    template<typename T>
    using ShrdPtrParam = const SharedPtr<T>&;

    using String = std::string;
    using StrParam = const String&;

    using StringView = std::string_view;
    using StrViewParam = const StringView&;

    template<typename T>
    using UniquePtr = std::unique_ptr<T>;
    template<typename T>
    using UniqPtrParam = const UniquePtr<T>&;

    template<typename... Types>
    using Variant = std::variant<Types...>;
    template<typename... Types>
    using VarParam = const Variant<Types...>&;

    template<typename T>
    using WeakPtr = std::weak_ptr<T>;
    template<typename T>
    using WeakPtrParam = const WeakPtr<T>&;

    using Wstring = std::wstring;
    using WstrParam = const Wstring&;

    using WstringView = std::wstring_view;
    using WstrViewParam = const WstringView&;
}

///////////////////////////
// Windows & DirectX SDK //
///////////////////////////

// Refer to https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#header-include-order
// It is recommended to include Agility SDK headers before the Windows 10 SDK to avoid conflicts.
#include <d3d12.h>
#include "d3dx12/d3dx12.h"

// Refer to https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#2.-set-agility-sdk-parameters
// We need to set Agility SDK parameters to help locate the custom D3D12Core.dll during building.
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 615; }
#ifdef _WIN64
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = "Lib/DirectX/x64/"; }
#else
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = "Lib/DirectX/x86/"; }
#endif

#include <comdef.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1effects_2.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <dwmapi.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <imm.h>
#include <wincodec.h>
#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "DirectXShaderCompiler/dxcapi.h"

namespace d14engine
{
    using namespace DirectX;

    using Microsoft::WRL::ComPtr;
    template<typename T>
    using ComPtrParam = const ComPtr<T>&;
}

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "WindowsCodecs.lib")

#pragma comment(lib, "dxcompiler.lib")

///////////////////
// Miscellaneous //
///////////////////

#if __cplusplus <= 202002L // "uz" was introduced in C++23
#pragma warning(push)
// warning C4455: literal suffix identifiers that
// do not start with an underscore are reserved
#pragma warning(disable : 4455)
constexpr size_t operator""uz(unsigned long long num)
{
    return (size_t)num;
}
#pragma warning(pop) // simply falls to std::uz when >= C++23
#endif

// Allows virtual inheritance to solve diamond problem.
#pragma warning(disable : 4250)
