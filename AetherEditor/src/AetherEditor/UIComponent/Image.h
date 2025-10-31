#pragma once
#include <Imgui/Core/imgui.h>
#include <Render/Render.h>
#include <Core/Core.h>
namespace AetherEditor::ImGuiComponent
{
    class Image
    {
    public:
        static std::expected<Image,std::string> Create(Aether::DeviceTexture& texture);
        ImTextureID GetTextureId() const
        {
            return m_TextureId;
        }
        ImVec2 GetSize() const
        {
            return m_Size;
        }
    private:
        Image()=default;
        Aether::DeviceSampler m_TextureSampler;
        ImTextureID m_TextureId;
        ImVec2 m_Size;
    };
}