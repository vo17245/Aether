#pragma once
#include <Imgui/Core/imgui.h>
#include <Render/Render.h>
#include <expected>
#include "Image.h"
namespace Aether::ImGuiComponent
{

class ImageView
{
public:
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;
    ImageView(ImageView&&) = delete;
    ImageView& operator=(ImageView&&) = delete;

    ImageView(Borrow<Image> image) : m_Image(image), m_Size(image->GetSize())
    {
    }
    void Draw()
    {
        ImGui::Image(m_Image->GetTextureId(), m_Size);
    }
    void SetSize(const ImVec2& size)
    {
        m_Size = size;
    }

private:
    Borrow<Image> m_Image;
    ImVec2 m_Size;
};
} // namespace Aether::ImGuiComponent