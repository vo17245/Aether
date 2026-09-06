#include <Render/RHI/Texture2D.h>
#include <Render/RHI/Backend/Vulkan/GlobalRenderContext.h>
#include <Render/RHI/Backend/Vulkan/ImageView.h>

namespace Aether::rhi
{
Texture2D Texture2D::Create(const TextureDesc& desc)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto texture = vk::Texture2D::Create(desc.width, desc.height, desc.pixelFormat,
                                             RHITextureUsageFlagsToVk(desc.usages), VK_IMAGE_LAYOUT_UNDEFINED);
        if (!texture)
        {
            assert(false && "Failed to create texture");
            return Texture2D();
        }
        auto result = Texture2D(std::move(texture.value()));
        if (desc.layout != TextureLayout::Undefined)
        {
            result.SyncTransitionLayout(TextureLayout::Undefined, desc.layout);
        }
        return result;
    }
    default:
        assert(false && "Not implemented");
        return Texture2D();
    }
}

TextureView Texture2D::CreateImageView(const TextureViewDesc& desc) const
{
    (void)desc;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto imageView = vk::ImageView::Create(std::get<vk::Texture2D>(m_Texture));
        if (!imageView)
        {
            assert(false && "Failed to create image view");
            return TextureView();
        }
        return TextureView(std::move(imageView.value()));
    }
    default:
        assert(false && "Not implemented");
        return TextureView();
    }
}
void Texture2D::SyncTransitionLayout(TextureLayout oldLayout, TextureLayout newLayout)
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
