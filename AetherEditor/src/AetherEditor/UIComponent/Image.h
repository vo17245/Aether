#pragma once
#include <Imgui/ImGui.h>
#include <Render/Render.h>
#include <Core/Core.h>
using namespace Aether;
namespace AetherEditor::ImGuiComponent
{
class Image
{
public:
    template <typename T>
        requires std::is_same_v<std::decay_t<T>, Ref<DeviceTexture>>
    static std::expected<Image, std::string> Create(T&& texture)
    {
        Image img;
        if (Aether::Render::Config::RenderApi == Aether::Render::Api::Vulkan)
        {
            img.m_TextureSampler = Aether::DeviceSampler::CreateDefault();

            img.m_TextureId = (ImTextureID)ImGui_ImplVulkan_AddTexture(
                img.m_TextureSampler.GetVk().GetHandle(), texture->GetOrCreateDefaultImageView().GetVk().GetHandle(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        else
        {
            assert(false && "Not implemented");
            return std::unexpected<std::string>("Not implemented");
        }
        img.m_Size.x = (float)texture->GetWidth();
        img.m_Size.y = (float)texture->GetHeight();
        img.m_Texture = std::forward<T>(texture);
        return img;
    }
    ImTextureID GetTextureId() const
    {
        return m_TextureId;
    }
    ImVec2 GetSize() const
    {
        return m_Size;
    }

private:
    Image() = default;
    Aether::DeviceSampler m_TextureSampler;
    ImTextureID m_TextureId;
    ImVec2 m_Size;
    Ref<DeviceTexture> m_Texture;
};
} // namespace AetherEditor::ImGuiComponent