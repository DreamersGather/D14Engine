﻿#include "Common/Precompile.h"

#include "Renderer/GraphUtils/Shader.h"

#include "Common/DirectXError.h"

#include "Renderer/GraphUtils/ParamHelper.h"

namespace d14engine::renderer::graph_utils
{
    namespace shader
    {
        ComPtr<IDxcUtils> g_utils;
        ComPtr<IDxcCompiler3> g_compiler;
        ComPtr<IDxcIncludeHandler> g_defaultIncludeHandler;

        void initialize()
        {
            THROW_IF_FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&g_utils)));
            THROW_IF_FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&g_compiler)));
            THROW_IF_FAILED(g_utils->CreateDefaultIncludeHandler(&g_defaultIncludeHandler));
        }

        ComPtr<IDxcBlob> load(WstrParam csoFileName)
        {
            ComPtr<IDxcBlobEncoding> cso;
            THROW_IF_FAILED(g_utils->LoadFile(csoFileName.c_str(), nullptr, &cso));
            return cso; // no encoding for binary data
        }

        void save(WstrParam csoFileName, IDxcBlob* blob)
        {
            auto hFile = CreateFile
            (
                /* lpFileName            */ csoFileName.c_str(),
                /* dwDesiredAccess       */ GENERIC_WRITE,
                /* dwShareMode           */ 0,
                /* lpSecurityAttributes  */ nullptr,
                /* dwCreationDisposition */ CREATE_ALWAYS,
                /* dwFlagsAndAttributes  */ FILE_ATTRIBUTE_NORMAL,
                /* hTemplateFile         */ nullptr
            );
            THROW_IF_TRUE(hFile == INVALID_HANDLE_VALUE);

            auto result = WriteFile
            (
                /* hFile                  */ hFile,
                /* lpBuffer               */ blob->GetBufferPointer(),
                /* nNumberOfBytesToWrite  */ (DWORD)blob->GetBufferSize(),
                /* lpNumberOfBytesWritten */ nullptr,
                /* lpOverlapped           */ nullptr
            );
            THROW_IF_FALSE(result);

            THROW_IF_FALSE(CloseHandle(hFile));
        }

        ComPtr<IDxcBlob> compile(
            WstrParam hlslFileName,
            const CompileOption& option)
        {
            ComPtr<IDxcBlobEncoding> hlsl;
            THROW_IF_FAILED(g_utils->LoadFile(hlslFileName.c_str(), nullptr, &hlsl));

            DxcBuffer source = {};
            source.Ptr = hlsl->GetBufferPointer();
            source.Size = hlsl->GetBufferSize();
            source.Encoding = DXC_CP_ACP;

            LPCWSTR arguments[] =
            {
                hlslFileName.c_str(),
                L"-E", option.entryPoint.c_str(),
                L"-T", option.targetProfile.c_str(),
#ifdef _DEBUG
                DXC_ARG_DEBUG,
                DXC_ARG_SKIP_OPTIMIZATIONS,
#else
                DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
                // DirectX API (e.g. DirectXMath) typically uses row-major matrix.
                // HLSL, however, rebelliously uses column-major matrix by default.
                // It is always better to specify the major-type explicitly anyway.
                DXC_ARG_PACK_MATRIX_ROW_MAJOR
            };

            ComPtr<IDxcResult> result;
            THROW_IF_FAILED(g_compiler->Compile(
                &source,
                ARR_NUM_ARGS(arguments),
                g_defaultIncludeHandler.Get(),
                IID_PPV_ARGS(&result)));

#ifdef _DEBUG
            ComPtr<IDxcBlobUtf8> error;

#pragma warning(push)
// This warning will be raised if an annotated function parameter is passed
// an unexpected value.  In the official example on GitHub the 4th parameter
// of IDxcResult::GetOutput is discarded (i.e. null ppOutputName), though it
// is annotated as _COM_Outptr_ instead of _COM_Outptr_opt_result_maybenull_.
#pragma warning(disable : 6387)

            THROW_IF_FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error), nullptr));

#pragma warning(pop)

            if (error && error->GetStringLength() != 0)
            {
                OutputDebugStringA((char*)error->GetStringPointer());
            }
#endif
            HRESULT hrStatus;
            THROW_IF_FAILED(result->GetStatus(&hrStatus));
            THROW_IF_FAILED(hrStatus);

            ComPtr<IDxcBlob> shader;

#pragma warning(push)
// Please refer to the above comment; the same warning applies here as well.
#pragma warning(disable : 6387)

            THROW_IF_FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));

#pragma warning(pop)

            return shader;
        }

        Object::Object(const CompileOption& option) : option(option) {}

        void processDefaultObject(WstrParam path, WstrParam name, ProcessOption option, Package& shaders)
        {
            if (option.in.has_value())
            {
                switch (option.in.value())
                {
                case CSO:
                {
                    auto csoPath = path + L"CSO/" + name;
                    for (auto& s : shaders)
                    {
                        s.second.blob = load(csoPath + L"_" + s.first + L".cso");
                    }
                    break;
                }
                case HLSL:
                {
                    auto hlslPath = path + L"HLSL/" + name;
                    for (auto& s : shaders)
                    {
                        auto& option = s.second.option;
                        THROW_IF_FALSE(option.has_value());
                        s.second.blob = compile(hlslPath + L".hlsl", option.value());
                    }
                    break;
                }
                default: break;
                }
            }
            if (option.out.has_value())
            {
                switch (option.out.value())
                {
                case CSO:
                {
                    auto csoPath = path + L"CSO/" + name;
                    for (auto& s : shaders)
                    {
                        save(csoPath + L"_" + s.first + L".cso", s.second.blob.Get());
                    }
                    break;
                }
                case HLSL:
                {
                    break; // HLSL can only be used as input data
                }
                default: break;
                }
            }
        }
    }
}
