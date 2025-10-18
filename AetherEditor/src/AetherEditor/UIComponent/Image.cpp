#include "Image.h"
#include <ImGui/Compat/ImGuiBackend.h>
namespace Aether::ImGuiComponent
{
std::expected<Image, std::string> Image::Create(DeviceTexture& texture)
{
    Image img;
    if (Render::Config::RenderApi == Render::Api::Vulkan)
    {
        img.m_TextureSampler = DeviceSampler::CreateDefault();

        img.m_TextureId = (ImTextureID)ImGui_ImplVulkan_AddTexture(img.m_TextureSampler.GetVk().GetHandle(),
                                                               texture.GetOrCreateDefaultImageView().GetVk().GetHandle(),
                                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    else
    {
        assert(false && "Not implemented");
        return std::unexpected<std::string>("Not implemented");
    }
    img.m_Size.x = (float)texture.GetWidth();
    img.m_Size.y = (float)texture.GetHeight();
    return img;

}
} // namespace Aether::ImGuiComponent