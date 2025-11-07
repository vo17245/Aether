#pragma once
#include <Core/Core.h>
#include <IO/Image.h>
#include <Render/Render.h>
#include <AetherEditor/UIComponent/Image.h>
#include <AetherEditor/UIComponent/ImageView.h>

using namespace Aether;
namespace AetherEditor::UI
{

    class TextureViewPanel
    {
    public:
        void OnImGuiUpdate();
        /**
         * @return error message if failed
        */
        std::optional<std::string> SetImageAddress(const std::string& address);
    private:
        std::string m_ImageAddress;
        Scope<ImGuiComponent::Image> m_Image;
        DeviceTexture m_Texture;
        std::optional<std::string> m_ErrorMessage;
        ImGuiComponent::ImageView m_ImageView;
    };
}