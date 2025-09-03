#pragma once
#include <Imgui/Core/imgui.h>
#include <Render/Render.h>
namespace Aether::ImGuiComponent
{
    class Image
    {
    public:
        static std::expected<Image,std::string> Create(DeviceTexture& texture);
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
        DeviceSampler m_TextureSampler;
        ImTextureID m_TextureId;
        ImVec2 m_Size;
    };
}