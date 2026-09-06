#pragma once
#include <Core/Core.h>
#include <Imgui/ImGui.h>
#include <Render/RHI.h>
#include <expected>
#include <type_traits>

using namespace Aether;
namespace AetherEditor::ImGuiComponent
{
class Image
{
public:
    template <typename T>
        requires std::is_same_v<std::decay_t<T>, Ref<rhi::Texture2D>>
    static std::expected<Image, std::string> Create(T&& texture)
    {
        if (!texture)
        {
            return std::unexpected<std::string>("texture is null");
        }

        Image img;
        img.m_Size.x = static_cast<float>(texture->GetWidth());
        img.m_Size.y = static_cast<float>(texture->GetHeight());
        img.m_Texture = std::forward<T>(texture);

        if (Aether::Render::Config::RenderApi == Aether::Render::Api::Vulkan)
        {
            img.m_TextureSampler = Aether::rhi::Sampler::CreateDefault();
            img.m_TextureView = img.m_Texture->CreateImageView({});
            img.m_TextureId = ImGui_ImplRenderGraph_AddTexture(
                *img.m_Texture, img.m_TextureView, img.m_TextureSampler);
        }
        else
        {
            return std::unexpected<std::string>("Unsupported render API");
        }
        return img;
    }

    Image(Image&& other) noexcept
        : m_TextureSampler(std::move(other.m_TextureSampler)), m_TextureView(std::move(other.m_TextureView)),
          m_TextureId(other.m_TextureId), m_Size(other.m_Size), m_Texture(std::move(other.m_Texture))
    {
        other.m_TextureId = 0;
    }
    Image& operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            m_TextureSampler = std::move(other.m_TextureSampler);
            m_TextureView = std::move(other.m_TextureView);
            m_TextureId = other.m_TextureId;
            m_Size = other.m_Size;
            m_Texture = std::move(other.m_Texture);
            other.m_TextureId = 0;
        }
        return *this;
    }

    ImTextureID GetTextureId() const
    {
        return m_TextureId;
    }
    const ImVec2& GetSize() const
    {
        return m_Size;
    }

private:
    Image() = default;

    Aether::rhi::Sampler m_TextureSampler;
    Aether::rhi::TextureView m_TextureView;
    ImTextureID m_TextureId = 0;
    ImVec2 m_Size{0.0f, 0.0f};
    Ref<rhi::Texture2D> m_Texture;
};
} // namespace AetherEditor::ImGuiComponent
