#include "Common/Precompile.h"

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

        ComPtr<IDxcBlob> compile(
            WstrParam fileName,
            WstrParam entryPoint,
            WstrParam targetProfile)
        {
            ComPtr<IDxcBlobEncoding> file;
            THROW_IF_FAILED(g_utils->LoadFile(fileName.c_str(), nullptr, &file));

            DxcBuffer source = {};
            source.Ptr = file->GetBufferPointer();
            source.Size = file->GetBufferSize();
            source.Encoding = DXC_CP_ACP;

            LPCWSTR arguments[] =
            {
                fileName.c_str(),
                L"-E", entryPoint.c_str(),
                L"-T", targetProfile.c_str(),
#ifdef _DEBUG
                DXC_ARG_DEBUG,
                DXC_ARG_SKIP_OPTIMIZATIONS
#else
                DXC_ARG_OPTIMIZATION_LEVEL3
#endif
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
            THROW_IF_FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));

            return shader;
        }
    }
}
