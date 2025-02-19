﻿#pragma once

#include "Common/Precompile.h"

namespace d14engine::renderer::graph_utils
{
    namespace shader
    {
        void initialize();

        ComPtr<IDxcBlob> load(WstrParam fileName);
        void save(WstrParam fileName, IDxcBlob* blob);

        struct CompileOption
        {
            Wstring entryPoint;
            Wstring targetProfile;
#ifdef _DEBUG
            bool debug = true;
#else
            bool debug = false;
#endif
            bool pdb = false;
            Wstring pdbOutPath;
        };
        ComPtr<IDxcBlob> compile(
            WstrParam hlslFileName,
            const CompileOption& option);

        enum class Format
        {
            CSO, // Compiled Shader Object
            HLSL // High Level Shader Language
        };
        constexpr static auto CSO = Format::CSO;
        constexpr static auto HLSL = Format::HLSL;

        struct StreamOption
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
        
        void loadDefaultObject(
            WstrParam path, WstrParam name,
            StreamOption option, Package& shaders);
    }
}
