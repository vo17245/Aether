#pragma once
#include <AetherEditor/UIComponent/ImageView.h>
#include <Render/RHI.h>
using namespace Aether;
namespace AetherEditor::UI
{

class ScenePanel
{
public:
    void SetTexture(Ref<rhi::Texture2D> texture)
    {
        auto img = ImGuiComponent::Image::Create(std::move(texture));
        if (!img)
        {
            m_Image.reset();
            m_ImageView.reset();
            return;
        }
        m_Image = CreateScope<ImGuiComponent::Image>(std::move(img.value()));
        auto view = CreateScope<ImGuiComponent::ImageView>(m_Image.get());
        m_ImageView = std::move(view);
    }
    void Draw()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(m_Title.c_str());

        ImGui::PopStyleVar();
        if (m_ImageView)
        {
            m_ImageView->Draw();
        }
        ImGui::End();
    }
    void SetTitle(const std::string& title)
    {
        m_Title = title;
    }

private:
    std::string m_Title = "Scene Panel";
    Scope<ImGuiComponent::Image> m_Image;
    Scope<ImGuiComponent::ImageView> m_ImageView;
};
}
