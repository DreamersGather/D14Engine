﻿#include "Common/Precompile.h"

#include "Renderer/Letterbox.h"

#include "Common/DirectXError.h"

#include "Renderer/GpuBuffer.h"
#include "Renderer/GraphUtils/Barrier.h"
#include "Renderer/GraphUtils/ParamHelper.h"
#include "Renderer/GraphUtils/PSO.h"
#include "Renderer/GraphUtils/Shader.h"
#include "Renderer/GraphUtils/StaticSampler.h"
#include "Renderer/Renderer.h"

namespace d14engine::renderer
{
    Letterbox::Letterbox(Renderer* rndr, Token) : rndr(rndr)
    {
        for (auto& cmdAlloc : m_cmdAllocs)
        {
            THROW_IF_FAILED(rndr->d3d12Device()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc)));
        }
    }

    bool Letterbox::enabled() const
    {
        return m_enabled;
    }

    void Letterbox::setEnabled(bool value)
    {
        m_enabled = value;
        if (m_enabled)
        {
            createRootSignature();
            createPipelineState();
            createVertexBuffer();
        }
        else // Clear created components.
        {
            m_rootSigature.Reset();
            m_pipelineState.Reset();

            m_vertexBuffer.reset();
            m_vertexBufferView = {};
        }
    }

    void Letterbox::resize(UINT sceneWidth, UINT sceneHeight, UINT windowWidth, UINT windowHeight)
    {
        float horzRatio = sceneWidth / (float)windowWidth;
        float vertRatio = sceneHeight / (float)windowHeight;

        float x = 1.0f;
        float y = 1.0f;

        if (horzRatio < vertRatio)
        {
            x = horzRatio / vertRatio;
        }
        else // fit width
        {
            y = vertRatio / horzRatio;
        }

        m_viewport.TopLeftX = windowWidth * (1.0f - x) * 0.5f;
        m_viewport.TopLeftY = windowHeight * (1.0f - y) * 0.5f;
        m_viewport.Width = x * windowWidth;
        m_viewport.Height = y * windowHeight;
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;

        m_scissors.left = (LONG)m_viewport.TopLeftX;
        m_scissors.right = (LONG)(m_viewport.TopLeftX + m_viewport.Width);
        m_scissors.top = (LONG)m_viewport.TopLeftY;
        m_scissors.bottom = (LONG)(m_viewport.TopLeftY + m_viewport.Height);
    }

    void Letterbox::createRootSignature()
    {
        CD3DX12_ROOT_PARAMETER1 rootParam = {};
        CD3DX12_DESCRIPTOR_RANGE1 descRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        rootParam.InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags =
        (
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |

            // We only need VS and PS to perform the resampling.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
        );
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc = {};
        rootSigDesc.Init_1_1(1, &rootParam, 1, &graph_utils::static_sampler::linearBorder(0), rootSigFlags);

        auto& maxVersion = rndr->d3d12DeviceInfo().feature.rootSignature.HighestVersion;

        ComPtr<ID3DBlob> rootSigBlob;
        THROW_IF_ERROR(D3DX12SerializeVersionedRootSignature(&rootSigDesc, maxVersion, &rootSigBlob, &error));
        THROW_IF_FAILED(rndr->d3d12Device()->CreateRootSignature(0, BLB_PSZ_ARGS(rootSigBlob), IID_PPV_ARGS(&m_rootSigature)));
    }

    void Letterbox::createPipelineState()
    {
        auto shaderPath = rndr->createInfo.binaryPath + L"Shaders/Letterbox.hlsl";

        auto vs = graph_utils::shader::compile(shaderPath, L"VS", L"vs_6_0");
        auto ps = graph_utils::shader::compile(shaderPath, L"PS", L"ps_6_0");

        D3D12_INPUT_ELEMENT_DESC inputElemDescs[] =
        {
            {
                /* SemanticName         */ "POSITION",
                /* SemanticIndex        */ 0,
                /* Format               */ DXGI_FORMAT_R32G32B32A32_FLOAT,
                /* InputSlot            */ 0,
                /* AlignedByteOffset    */ 0,
                /* InputSlotClass       */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                /* InstanceDataStepRate */ 0
            },
            {
                /* SemanticName         */ "TEXCOORD",
                /* SemanticIndex        */ 0,
                /* Format               */ DXGI_FORMAT_R32G32_FLOAT,
                /* InputSlot            */ 0,
                /* AlignedByteOffset    */ D3D12_APPEND_ALIGNED_ELEMENT,
                /* InputSlotClass       */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                /* InstanceDataStepRate */ 0
            }
        };
        auto psoDesc = graph_utils::graphicsPipelineStateDescTemplate();

        psoDesc.pRootSignature = m_rootSigature.Get();
        psoDesc.VS = { BLB_PSZ_ARGS(vs) };
        psoDesc.PS = { BLB_PSZ_ARGS(ps) };
        psoDesc.InputLayout = { ARR_NUM_ARGS(inputElemDescs) };
        psoDesc.DepthStencilState.DepthEnable = FALSE;

        THROW_IF_FAILED(rndr->d3d12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    void Letterbox::createVertexBuffer()
    {
        Vertex quad[] =
        {
            /* Top    Left  */ { { -1.0f, +1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
            /* Top    Right */ { { +1.0f, +1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
            /* Bottom Left  */ { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
            /* Bottom Right */ { { +1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }
        };
        m_vertexBuffer = std::make_unique<DefaultBuffer>(rndr->d3d12Device(), sizeof(quad));
        m_vertexBuffer->uploadData(rndr->cmdList(), quad, sizeof(quad));

        m_vertexBufferView.BufferLocation = m_vertexBuffer->resource()->GetGPUVirtualAddress();
        m_vertexBufferView.SizeInBytes = sizeof(quad);
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    }

    void Letterbox::present()
    {
        auto& cmdAlloc = m_cmdAllocs.at(rndr->currFrameIndex());

        THROW_IF_FAILED(cmdAlloc->Reset());
        THROW_IF_FAILED(rndr->cmdList()->Reset(cmdAlloc.Get(), nullptr));

        if (m_enabled)
        {
            postSceneToBackBuffer();
        }
        else copySceneToBackBuffer();

        rndr->submitCmdList();
    }

    void Letterbox::copySceneToBackBuffer()
    {
        D3D12_RESOURCE_BARRIER barriers[] =
        {
            CD3DX12_RESOURCE_BARRIER::Transition(
                rndr->currBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_COPY_DEST),

            CD3DX12_RESOURCE_BARRIER::Transition(
                rndr->sceneBuffer(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_SOURCE),
        };
        rndr->cmdList()->ResourceBarrier(NUM_ARR_ARGS(barriers));

        CD3DX12_TEXTURE_COPY_LOCATION dst(rndr->currBackBuffer());
        CD3DX12_TEXTURE_COPY_LOCATION src(rndr->sceneBuffer());
        CD3DX12_BOX srcBox(0, 0, rndr->getSceneWidth(), rndr->getSceneHeight());

        rndr->cmdList()->CopyTextureRegion(&dst, 0, 0, 0, &src, &srcBox);

        graph_utils::revertBarrier(NUM_ARR_ARGS(barriers));
        rndr->cmdList()->ResourceBarrier(NUM_ARR_ARGS(barriers));
    }

    void Letterbox::postSceneToBackBuffer()
    {
        D3D12_RESOURCE_BARRIER barriers[] =
        {
            CD3DX12_RESOURCE_BARRIER::Transition(
                rndr->currBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET),

            CD3DX12_RESOURCE_BARRIER::Transition(
                rndr->sceneBuffer(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        };
        rndr->cmdList()->ResourceBarrier(NUM_ARR_ARGS(barriers));

        rndr->cmdList()->RSSetViewports(1, &m_viewport);
        rndr->cmdList()->RSSetScissorRects(1, &m_scissors);

        auto rtvHandle = rndr->backRtvHandle();

        rndr->cmdList()->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);
        rndr->cmdList()->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

        rndr->cmdList()->SetGraphicsRootSignature(m_rootSigature.Get());
        rndr->cmdList()->SetPipelineState(m_pipelineState.Get());

        ID3D12DescriptorHeap* ppHeaps[] = { rndr->srvHeap() };
        rndr->cmdList()->SetDescriptorHeaps(NUM_ARR_ARGS(ppHeaps));

        rndr->cmdList()->SetGraphicsRootDescriptorTable(
            0, rndr->srvHeap()->GetGPUDescriptorHandleForHeapStart());

        // Use "TRIANGLESTRIP" instead of "TRIANGLELIST" to fit the vertex data.
        rndr->cmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        rndr->cmdList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);

        rndr->cmdList()->DrawInstanced(4, 1, 0, 0);

        graph_utils::revertBarrier(NUM_ARR_ARGS(barriers));
        rndr->cmdList()->ResourceBarrier(NUM_ARR_ARGS(barriers)); 
    }
}
