#include "Common/Precompile.h"

#include "UIKit/ScenePanel.h"

#include "Common/DirectXError.h"
#include "Common/MathUtils/2D.h"

#include "Renderer/GraphUtils/Barrier.h"

#include "UIKit/Application.h"
#include "UIKit/BitmapObject.h"

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

        target[m_primaryLayer = std::make_shared<Layer>()] = {};
    }

    void ScenePanel::onInitializeFinish()
    {
        Panel::onInitializeFinish();

        auto rndr = Application::g_app->dxRenderer();

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        auto device = rndr->d3d12Device();
        THROW_IF_FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)));

        loadOffscreenTextures();

        m_primaryLayer->f_onRendererDrawD3d12Layer = [this]
        (Layer* layer, Renderer* rndr)
        {
            auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
                m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), 0,
                (UINT)rndr->d3d12DeviceInfo().property.descHandleIncrementSize.RTV);

            rndr->cmdList()->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);
            rndr->cmdList()->ClearRenderTargetView(rtvHandle, offscreenTexture.color, 0, nullptr);
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

    void ScenePanel::loadOffscreenTextures()
    {
    }

    ID3D12DescriptorHeap* ScenePanel::rtvHeap() const
    {
        return m_rtvHeap.Get();
    }

    ID3D12Resource* ScenePanel::buffer() const
    {
        return m_buffer.Get();
    }

    ID3D11Resource* ScenePanel::wrapper() const
    {
        return m_wrapper.Get();
    }

    ID2D1Bitmap1* ScenePanel::renderTarget() const
    {
        return m_renderTarget.Get();
    }

    void ScenePanel::onRendererDrawD2d1ObjectHelper(Renderer* rndr)
    {
        Panel::onRendererDrawD2d1ObjectHelper(rndr);

        rndr->d2d1DeviceContext()->DrawBitmap(
            m_renderTarget.Get(),
            math_utils::roundf(m_absoluteRect),
            offscreenTexture.opacity,
            offscreenTexture.interpolationMode.has_value() ?
            offscreenTexture.interpolationMode.value() :
            BitmapObject::g_interpolationMode);
    }

    void ScenePanel::onSizeHelper(SizeEvent& e)
    {
        Panel::onSizeHelper(e);

        loadOffscreenTextures();
    }
}
