#include "CreateTexturePanel.h"
#include <UI/FileDialog.h>
namespace AetherEditor::UI
{
    void CreateTexturePanel::OnImGuiUpdate()
    {
        if (!m_Visible)
            return;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoDocking;
        ImGui::Begin(m_Title.c_str(),nullptr,window_flags);
        ImGui::InputText("FilePath", m_FilePathBuffer, sizeof(m_FilePathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Browse"))
        {
            auto file = Aether::UI::SyncSelectFile();
            if (file)
            {
                memcpy(m_FilePathBuffer, file->c_str(), std::min(file->size() + 1, sizeof(m_FilePathBuffer)));
            }
        }
        ImGui::Text("%s",m_AddressDisplay.c_str());
        if(ImGui::InputText("Name", m_NameBuffer, sizeof(m_NameBuffer)))
        {
            m_Address = m_Folder+std::string(m_NameBuffer);
            m_AddressDisplay = "Address: " + m_Address;
        }
        ImGui::Checkbox("sRGB", &m_SRGB);
        if(ImGui::Button("Create"))
        {
            Aether::Project::ImportTextureParams params;
            params.FilePath = std::string(m_FilePathBuffer);
            params.Name = std::string(m_NameBuffer);
            params.Address = m_Address;
            params.sRGB = m_SRGB;
            TextureCreateEventHandler.Broadcast(params);
            m_Visible = false;
        }
        ImGui::End();
    }
}