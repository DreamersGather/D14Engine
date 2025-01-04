#pragma once

#include "Common/Precompile.h"

#include "Common/MathUtils/3D.h"

namespace d14engine::pipeline
{
    struct Transformation
    {
        XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 scaling = { 1.0f, 1.0f, 1.0f };

        XMFLOAT4X4 worldMatrix = math_utils::identityFloat4x4();

        void updateWorldMatrix();
    };
}
