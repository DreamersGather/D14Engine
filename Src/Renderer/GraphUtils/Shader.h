#pragma once

#include "Common/Precompile.h"

namespace d14engine::renderer::graph_utils
{
    namespace shader
    {
        void initialize();

        ComPtr<IDxcBlob> compile(
            WstrParam fileName,
            WstrParam entryPoint,
            WstrParam targetProfile);
    }
}
