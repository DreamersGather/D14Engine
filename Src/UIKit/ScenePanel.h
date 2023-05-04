#pragma once

#include "Common/Precompile.h"

#include "Renderer/Interfaces/IDrawLayer.h"
#include "Renderer/Interfaces/IDrawObject.h"

#include "UIKit/Panel.h"

namespace d14engine::uikit
{
    struct ScenePanel : Panel
    {
        ScenePanel(const D2D1_RECT_F& rect = {}, int cmdLayerPriority = 100);

        using DrawObjectSet = renderer::Renderer::CommandLayer::DrawObjectSet;

        void onInitializeFinish() override;

        struct Layer : renderer::IDrawLayer
        {
            void onRendererUpdateLayer(renderer::Renderer* rndr) override;

            Function<void(Layer*, renderer::Renderer*)> f_onRendererUpdateLayer = {};

            void onRendererDrawD3d12Layer(renderer::Renderer* rndr) override;

            Function<void(Layer*, renderer::Renderer*)> f_onRendererDrawD3d12Layer = {};

        protected:
            virtual void onRendererUpdateLayerHelper(renderer::Renderer* rndr);
            virtual void onRendererDrawD3d12LayerHelper(renderer::Renderer* rndr);

        protected:
            bool m_visible = true;

        public:
            bool isD3d12LayerVisible() const override;
            void setD3d12LayerVisible(bool value) override;
        };

        struct Object : renderer::IDrawObject
        {
            void onRendererUpdateObject(renderer::Renderer* rndr) override;

            Function<void(Object*, renderer::Renderer*)> f_onRendererUpdateObject = {};

            void onRendererDrawD3d12Object(renderer::Renderer* rndr) override;

            Function<void(Object*, renderer::Renderer*)> f_onRendererDrawD3d12Object = {};

        protected:
            virtual void onRendererUpdateObjectHelper(renderer::Renderer* rndr);
            virtual void onRendererDrawD3d12ObjectHelper(renderer::Renderer* rndr);

        protected:
            bool m_visible = true;

        public:
            bool isD3d12ObjectVisible() const override;
            void setD3d12ObjectVisible(bool value) override;
        };

    protected:
        SharedPtr<Layer> m_primaryLayer = {};

        SharedPtr<renderer::Renderer::CommandLayer> m_cmdLayer = {};

    public:
        const SharedPtr<Layer>& primaryLayer() const;

        const SharedPtr<renderer::Renderer::CommandLayer>& cmdLayer() const;

        int cmdLayerPriority() const;
        void setCmdLayerPriority(int value);

    protected:
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap = {};

        ComPtr<ID3D12Resource> m_buffer = {};

        ComPtr<ID3D11Resource> m_wrapper = {};

        ComPtr<ID2D1Bitmap1> m_renderTarget = {};

        void loadOffscreenTextures();

    public:
        ID3D12DescriptorHeap* rtvHeap() const;

        ID3D12Resource* buffer() const;

        ID3D11Resource* wrapper() const;

        ID2D1Bitmap1* renderTarget() const;

        struct OffscreenTexture
        {
            XMVECTORF32 color = Colors::Blue;

            float opacity = 1.0f;
            Optional<D2D1_INTERPOLATION_MODE> interpolationMode = std::nullopt;
        }
        offscreenTexture = {};

    protected:
        // IDrawObject2D
        void onRendererDrawD2d1ObjectHelper(renderer::Renderer* rndr) override;

        // Panel
        void onSizeHelper(SizeEvent& e) override;
    }; 
}
