#include "Common/Precompile.h"

#include "UIKit/ScenePanel.h"

#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"
#include "Common/MathUtils/Basic.h"

#include "Renderer/GraphUtils/Barrier.h"

#include "UIKit/Application.h"
#include "UIKit/BitmapObject.h"
#include "UIKit/PlatformUtils.h"

using namespace d14engine::renderer;

namespace d14engine::uikit
{
    ScenePanel::ScenePanel(const D2D1_RECT_F& rect, int cmdLayerPriority)
        :
        Panel(rect)
    {
        auto rndr = Application::g_app->dxRenderer();

        auto device = rndr->d3d12Device();
        m_cmdLayer = std::make_shared<Renderer::CommandLayer>(device);

        m_cmdLayer->setPriority(cmdLayerPriority);
        rndr->cmdLayers.insert(m_cmdLayer);

        using D3D12Target = Renderer::CommandLayer::D3D12Target;
        auto& target = m_cmdLayer->drawTarget.emplace<D3D12Target>();

        m_primaryLayer = std::make_shared<Layer>();
        m_primaryLayer->setPriority(INT_MIN);
        target[m_primaryLayer] = {};

        m_closingLayer = std::make_shared<Layer>();
        m_closingLayer->setPriority(INT_MAX);
        target[m_closingLayer] = {};
    }

    void ScenePanel::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        auto rndr = Application::g_app->dxRenderer();

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0; // single-adapter

        auto device = rndr->d3d12Device();
        THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)));

        loadOffscreenTexture();

        m_primaryLayer->f_onRendererDrawD3d12Layer = [this]
        (Layer* layer, Renderer* rndr)
        {
            auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

            rndr->cmdList()->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);
            rndr->cmdList()->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
        };
        m_closingLayer->f_onRendererDrawD3d12Layer = [this]
        (Layer* layer, Renderer* rndr)
        {
            if (m_msaaEnabled)
            {
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition
                (
                    m_msaaBuffer.Get(),
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    D3D12_RESOURCE_STATE_RESOLVE_SOURCE
                );
                rndr->cmdList()->ResourceBarrier(1, &barrier);

                rndr->cmdList()->ResolveSubresource(
                    m_backBuffer.Get(), 0,
                    m_msaaBuffer.Get(), 0,
                    Renderer::g_renderTargetFormat);

                graph_utils::revertBarrier(1, &barrier);
                rndr->cmdList()->ResourceBarrier(1, &barrier);
            }
        };
    }

    void ScenePanel::Layer::onRendererUpdateLayer(Renderer* rndr)
    {
        onRendererUpdateLayerHelper(rndr);

        if (f_onRendererUpdateLayer) f_onRendererUpdateLayer(this, rndr);
    }

    void ScenePanel::Layer::onRendererDrawD3d12Layer(Renderer* rndr)
    {
        onRendererDrawD3d12LayerHelper(rndr);

        if (f_onRendererDrawD3d12Layer) f_onRendererDrawD3d12Layer(this, rndr);
    }

    void ScenePanel::Layer::onRendererUpdateLayerHelper(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    void ScenePanel::Layer::onRendererDrawD3d12LayerHelper(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    bool ScenePanel::Layer::isD3d12LayerVisible() const
    {
        return m_visible;
    }

    void ScenePanel::Layer::setD3d12LayerVisible(bool value)
    {
        m_visible = value;
    }

    void ScenePanel::Object::onRendererUpdateObject(Renderer* rndr)
    {
        onRendererUpdateObjectHelper(rndr);

        if (f_onRendererUpdateObject) f_onRendererUpdateObject(this, rndr);
    }

    void ScenePanel::Object::onRendererDrawD3d12Object(Renderer* rndr)
    {
        onRendererDrawD3d12ObjectHelper(rndr);

        if (f_onRendererDrawD3d12Object) f_onRendererDrawD3d12Object(this, rndr);
    }

    void ScenePanel::Object::onRendererUpdateObjectHelper(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    void ScenePanel::Object::onRendererDrawD3d12ObjectHelper(Renderer* rndr)
    {
        // This method intentionally left blank.
    }

    bool ScenePanel::Object::isD3d12ObjectVisible() const
    {
        return m_visible;
    }

    void ScenePanel::Object::setD3d12ObjectVisible(bool value)
    {
        m_visible = value;
    }

    const SharedPtr<ScenePanel::Layer>& ScenePanel::primaryLayer() const
    {
        return m_primaryLayer;
    }

    const SharedPtr<ScenePanel::Layer>& ScenePanel::closingLayer() const
    {
        return m_closingLayer;
    }

    const SharedPtr<Renderer::CommandLayer>& ScenePanel::cmdLayer() const
    {
        return m_cmdLayer;
    }

    int ScenePanel::cmdLayerPriority() const
    {
        return m_cmdLayer->priority();
    }

    void ScenePanel::setCmdLayerPriority(int value)
    {
        auto rndr = Application::g_app->dxRenderer();
        rndr->cmdLayers.erase(m_cmdLayer);
        m_cmdLayer->setPriority(value);
        rndr->cmdLayers.insert(m_cmdLayer);
    }

    void ScenePanel::loadOffscreenTexture()
    {
        createBackBuffer();
        if (m_msaaEnabled)
        {
            createMsaaBuffer();
        }
        createWrappedBuffer();
    }

    void ScenePanel::createBackBuffer()
    {
        auto texSize = math_utils::roundu(size());
        texSize.width = std::max(texSize.width, 1u);
        texSize.height = std::max(texSize.height, 1u);

        auto rndr = Application::g_app->dxRenderer();

        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D
        (
            /* format    */ rndr->g_renderTargetFormat,
            /* width     */ math_utils::round<UINT64>(texSize.width),
            /* height    */ math_utils::round<UINT>(texSize.height),
            /* arraySize */ 1,
            /* mipLevels */ 1
        );
        if (m_msaaEnabled)
        {
            auto d3d12Device = rndr->d3d12Device();
            THROW_IF_FAILED(d3d12Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_RESOLVE_DEST,
                nullptr,
                IID_PPV_ARGS(&m_backBuffer)));
        }
        else // render target
        {
            desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = rndr->g_renderTargetFormat;
            memcpy(clearValue.Color, m_clearColor, 4 * sizeof(FLOAT));

            auto d3d12Device = rndr->d3d12Device();
            THROW_IF_FAILED(d3d12Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                &clearValue,
                IID_PPV_ARGS(&m_backBuffer)));

            auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
            d3d12Device->CreateRenderTargetView(m_backBuffer.Get(), nullptr, rtvHandle);
        }
    }

    void ScenePanel::createMsaaBuffer()
    {
        auto texSize = math_utils::roundu(size());
        texSize.width = std::max(texSize.width, 1u);
        texSize.height = std::max(texSize.height, 1u);

        auto rndr = Application::g_app->dxRenderer();

        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D
        (
            /* format    */ rndr->g_renderTargetFormat,
            /* width     */ math_utils::round<UINT64>(texSize.width),
            /* height    */ math_utils::round<UINT>(texSize.height),
            /* arraySize */ 1,
            /* mipLevels */ 1
        );
        desc.SampleDesc.Count = m_sampleCount;
        desc.SampleDesc.Quality = m_sampleQuality;
        desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = rndr->g_renderTargetFormat;
        memcpy(clearValue.Color, m_clearColor, 4 * sizeof(FLOAT));

        auto d3d12Device = rndr->d3d12Device();
        THROW_IF_FAILED(d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_msaaBuffer)));

        auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        d3d12Device->CreateRenderTargetView(m_msaaBuffer.Get(), nullptr, rtvHandle);
    }

    void ScenePanel::createWrappedBuffer()
    {
        auto rndr = Application::g_app->dxRenderer();
        auto dpi = platform_utils::dpi();

        D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1
        (
            /* bitmapOptions */ D2D1_BITMAP_OPTIONS_NONE,
            /* pixelFormat   */ D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
            /* dpiX          */ dpi,
            /* dpiY          */ dpi
        );
        if (!m_msaaEnabled)
        {
            props.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
        }
        D3D11_RESOURCE_FLAGS flags = {};
        flags.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        if (!m_msaaEnabled)
        {
            flags.BindFlags |= D3D11_BIND_RENDER_TARGET;
        }
        auto backBufferState = m_msaaEnabled ?
            D3D12_RESOURCE_STATE_RESOLVE_DEST :
            D3D12_RESOURCE_STATE_RENDER_TARGET;

        auto d3d11On12Device = rndr->d3d11On12Device();
        THROW_IF_FAILED(d3d11On12Device->CreateWrappedResource
        (
            /* pResource12  */ m_backBuffer.Get(),
            /* pFlags11     */ &flags,
            /* InState      */ backBufferState,
            /* OutState     */ backBufferState,
            /* ppResource11 */ IID_PPV_ARGS(&m_wrappedBuffer)
        ));
        ComPtr<IDXGISurface> surface;
        THROW_IF_FAILED(m_wrappedBuffer.As(&surface));

        auto context = rndr->d2d1DeviceContext();
        THROW_IF_FAILED(context->CreateBitmapFromDxgiSurface(surface.Get(), &props, &m_interpBitmap));
    }

    UINT ScenePanel::sampleCount() const
    {
        return m_sampleCount;
    }

    UINT ScenePanel::sampleQuality() const
    {
        return m_sampleQuality;
    }

    bool ScenePanel::setMultiSample(UINT count, OptParam<UINT> quality)
    {
        auto rndr = Application::g_app->dxRenderer();

        if (count <= 1)
        {
            m_msaaEnabled = false;

            m_sampleCount = 1;
            m_sampleQuality = 0;

            rndr->beginGpuCommand();
            loadOffscreenTexture();
            rndr->endGpuCommand();
            return true;
        }
        else // Enable MSAA
        {
            auto& feature = rndr->d3d12DeviceInfo().feature;
            auto level = feature.queryMsaaQualityLevel(count);
            if (level.has_value())
            {
                m_sampleCount = count;
                m_sampleQuality = level.value();

                m_msaaEnabled = true;

                rndr->beginGpuCommand();
                loadOffscreenTexture();
                rndr->endGpuCommand();
                return true;
            }
            else return false;
        }
    }

    bool ScenePanel::msaaEnabled() const
    {
        return m_msaaEnabled;
    }

    const XMVECTORF32& ScenePanel::clearColor() const
    {
        return m_clearColor;
    }

    void ScenePanel::setClearColor(const XMVECTORF32& color)
    {
        m_clearColor = color;

        auto rndr = Application::g_app->dxRenderer();
        rndr->beginGpuCommand();
        
        loadOffscreenTexture();

        rndr->endGpuCommand();
    }

    ID3D12DescriptorHeap* ScenePanel::rtvHeap() const
    {
        return m_rtvHeap.Get();
    }

    ID3D12Resource* ScenePanel::backBuffer() const
    {
        return m_backBuffer.Get();
    }

    ID3D11Resource* ScenePanel::wrappedBuffer() const
    {
        return m_wrappedBuffer.Get();
    }

    ID2D1Bitmap1* ScenePanel::interpBitmap() const
    {
        return m_interpBitmap.Get();
    }

    void ScenePanel::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        Panel::onRendererDrawD2d1ObjectHelper(rndr);

        auto& interpMode = bitmapSamplingSetting.interpolationMode;

        rndr->d2d1DeviceContext()->DrawBitmap(
            m_interpBitmap.Get(),
            math_utils::roundf(m_absoluteRect),
            bitmapSamplingSetting.opacity,
            interpMode.has_value() ? interpMode.value() :
            BitmapObject::g_interpolationMode);
    }

    void ScenePanel::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        auto rndr = Application::g_app->dxRenderer();
        rndr->beginGpuCommand();
        
        loadOffscreenTexture();

        rndr->endGpuCommand();
    }
}
