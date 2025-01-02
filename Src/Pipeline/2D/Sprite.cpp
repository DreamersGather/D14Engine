#include "Common/Precompile.h"

#include "Pipeline/2D/Sprite.h"

#include "Common/DirectXError.h"

#include "Renderer/GpuBuffer.h"
#include "Renderer/GraphUtils/Bitmap.h"
#include "Renderer/Renderer.h"

using namespace d14engine::renderer;

namespace d14engine::pipeline
{
    Sprite::Sprite(Renderer* rndr) : rndr(rndr)
    {
        //auto src = graph_utils::bitmap::load(imagePath);
        //auto data = graph_utils::bitmap::map(src.Get());

        //UINT size = 0;
        //BYTE* ptr = nullptr;
        //THROW_IF_FAILED(data->GetDataPointer(&size, &ptr));
        //m_texture = std::make_unique<DefaultBuffer>(rndr->d3d12Device(), size);
        //m_texture->uploadData(rndr->cmdList(), ptr, size);
    }

    void Sprite::onRendererUpdateObjectHelper(Renderer* rndr)
    {
    }

    void Sprite::onRendererDrawD3d12ObjectHelper(Renderer* rndr)
    {
    }
}
