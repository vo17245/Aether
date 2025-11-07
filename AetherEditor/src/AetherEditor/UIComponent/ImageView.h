#pragma once
#include <Imgui/Core/imgui.h>
#include <Render/Render.h>
#include <expected>
#include "Image.h"
namespace AetherEditor::ImGuiComponent
{

class ImageView
{
public:
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;
    ImageView(ImageView&&) = delete;
    ImageView& operator=(ImageView&&) = delete;
    ImageView()
    {
        m_Size = ImVec2(0, 0);
    }
    void Draw()
    {
        if (!m_Image)
        {
            return;
        }
        ImGui::Image(m_Image->GetTextureId(), m_Size);
    }
    void SetSize(const ImVec2& size)
    {
        m_Size = size;
    }
    void SetImage(Image* image)
    {
        if (image)
        {
            m_Image = image;
            m_Size = image->GetSize();
        }
        else
        {
            m_Image = nullptr;
            m_Size = ImVec2(0, 0);
        }
    }

private:
    Image* m_Image;
    ImVec2 m_Size;
};
} // namespace AetherEditor::ImGuiComponent