#include "CreateProjectPage.h"
#include <UI/FileDialog.h>
void CreateProjectPage::OnImGuiUpdate()
{

    ImGui::SetNextWindowPos(GetPos());
    ImGui::SetNextWindowSize(GetSize());
    ImGui::Begin("Create New Project", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_NoTitleBar);
    ImGui::InputText("Project Name", m_ProjectName, sizeof(m_ProjectName));
    ImGui::InputText("Project Location", m_ProjectLocation, sizeof(m_ProjectLocation));
    ImGui::SameLine();
    if(ImGui::Button("Browse"))
    {
        auto dirEx=UI::SyncSelectDirectroy();
    }
    ImGui::Separator();
    if (ImGui::Button("Create"))
    {
        m_Params.projectName = m_ProjectName;
    }
    ImGui::End();
}
ImVec2 CreateProjectPage::GetSize()
{
    return ImVec2(400, 200);
}
ImVec2 CreateProjectPage::GetPos()
{
    return ImVec2(300, 200);
}