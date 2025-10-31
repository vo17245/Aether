#pragma once
#include <AetherEditor/Panel/Panel.h>
namespace AetherEditor::UI
{
struct CreateFolderParams
{
    std::string FolderName;
};
class CreateFolderPanel : public Panel
{
public:
    CreateFolderPanel() : Panel("Create Folder")
    {
    }
    void OnImGuiUpdate()override
    {
        if (!m_Visible)
            return;
       
        ImGui::Begin(m_Title.c_str(), &m_Visible);
        ImGui::InputText("Folder Name", folderName, sizeof(folderName));
        if (ImGui::Button("Create"))
        {
            CreateFolderParams params;
            params.FolderName = std::string(folderName);
            folderName[0] = '\0'; // Clear input after creation
            m_Visible = false; // Close panel after creation
            OnCreateFolderEventHandler.Broadcast(params);
        }
        ImGui::End();
    }


private:
    char folderName[256] = "";
public:
    Delegate<void(const CreateFolderParams& params)> OnCreateFolderEventHandler;
};
} // namespace AetherEditor::UI