#pragma once

#include "Common/Precompile.h"

namespace d14engine::pipeline
{
    struct Vertex_SolidColor
    {
        XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        XMFLOAT4 color = XMFLOAT4{ Colors::Black };
    };
    struct Vertex_Texture2D
    {
        XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        XMFLOAT2 texcoord = { 0.0f, 0.0f };
    };
}
