#pragma once

#include "Common/Precompile.h"

#include "Renderer/Interfaces/DrawLayer.h"

namespace d14engine::renderer { struct DefaultBuffer; }

namespace d14engine::pipeline
{
    struct SpriteGroup : renderer::DrawLayer
    {
        explicit SpriteGroup(renderer::Renderer* rndr);

        // prevent std::unique_ptr from generating default deleter
        virtual ~SpriteGroup() = default;

        void onRendererDrawD3d12LayerHelper(renderer::Renderer* rndr) override;

    protected:
        ComPtr<ID3D12RootSignature> m_rootSigature = {};
        ComPtr<ID3D12PipelineState> m_pipelineState = {};

        struct Vertex
        {
            XMFLOAT3 position = {};
            XMFLOAT2 texcoord = {};
        };

        UniquePtr<renderer::DefaultBuffer> m_vertexBuffer = {};
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

    protected:
        void createRootSignature();
        void createPipelineState();
        void createVertexBuffer();
    };
}
