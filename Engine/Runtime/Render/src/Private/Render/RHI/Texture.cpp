#include <Render/RHI/Texture.h>
#include <Render/RHI/Backend/Vulkan/GlobalRenderContext.h>

namespace Aether::rhi
{
void Texture::SyncTransitionLayout(TextureLayout oldLayout, TextureLayout newLayout)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto& texture = std::get<vk::Texture2D>(m_Texture);
        texture.SyncTransitionLayout(vk::GRC::GetGraphicsCommandPool(), RHITextureLayoutToVk(oldLayout),
                                     RHITextureLayoutToVk(newLayout));
    }
    break;
    default:
        assert(false && "Not implemented");
        break;
    }
}
} // namespace Aether::rhi