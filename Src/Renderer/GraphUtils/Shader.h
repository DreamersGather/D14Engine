#pragma once

#include "Common/Precompile.h"

namespace d14engine::renderer::graph_utils
{
    namespace shader
    {
        void initialize();

        // CSO: Compiled Shader Object

        ComPtr<IDxcBlob> load(WstrParam csoFileName);
        void save(WstrParam csoFileName, IDxcBlob* blob);

        struct CompileOption
        {
            Wstring entryPoint;
            Wstring targetProfile;
        };
        ComPtr<IDxcBlob> compile(
            WstrParam hlslFileName,
            const CompileOption& option);

        enum class Format
        {
            CSO, HLSL
        };
        constexpr static auto CSO = Format::CSO;
        constexpr static auto HLSL = Format::HLSL;

        struct ProcessOption
        {
            Optional<Format> in;
            Optional<Format> out;
        };
        struct Object
        {
            ComPtr<IDxcBlob> blob;
            Optional<CompileOption> option;

            Object() = default;
            Object(const CompileOption& option);
        };
        using Package = std::unordered_map<Wstring, Object>;
        
        void processDefaultObject(
            WstrParam path, WstrParam name,
            ProcessOption option, Package& shaders);
    }
}
