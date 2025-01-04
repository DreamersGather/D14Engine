#include "Common/Precompile.h"

#include "Pipeline/Data/Transformation.h"

namespace d14engine::pipeline
{
    void Transformation::updateWorldMatrix()
    {
        auto positionVec3 = XMLoadFloat3(&position);
        auto rotationVec3 = XMLoadFloat3(&rotation);
        auto scalingVec3 = XMLoadFloat3(&scaling);

        XMStoreFloat4x4(&worldMatrix, XMMatrixTransformation
        (
            /* ScalingOrigin      */ XMVectorZero(),
            /* ScalingOrientation */ XMQuaternionIdentity(),
            /* Scaling            */ scalingVec3,
            /* RotationOrigin     */ XMVectorZero(),
            /* Rotation           */ XMQuaternionRotationRollPitchYawFromVector(rotationVec3),
            /* Translation        */ positionVec3)
        );
    }
}
