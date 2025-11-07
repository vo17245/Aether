#include "EntryPage.h"
#include <Project/Project.h>
#include <UI/FileDialog.h>
#include <AetherEditor/Global/GlobalProject.h>
namespace AetherEditor::UI
{
static std::expected<Scope<Aether::Project::Project>, std::string>
OpenExistingProject(const std::string& projectFilePath)
{
    auto loadResult = Aether::Project::Project::LoadFromFile(projectFilePath);
    if (!loadResult)
    {
        return std::unexpected<std::string>("Failed to load project file: " + loadResult.error());
    }
    return std::move(*loadResult);
}
void EntryPage::OnImGuiUpdate()
{
    ImGui::Begin("Welcome to Aether Editor!", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("Get Started:");
    if (ImGui::Button("Create New Project"))
    {
        GetPageRouter()->NavigateTo("CreateProjectPage");
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Existing Project"))
    {
        auto file=Aether::UI::SyncSelectFile();
        if(file)
        {
            auto openResult=OpenExistingProject(*file);
            if(!openResult)
            {
                LogE("Failed to open project: {}",openResult.error());
            }
            else
            {
                GlobalProject::SetProject(std::move(*openResult));
                GetPageRouter()->NavigateTo("MainPage");
            }
        }
    }
    ImGui::End();
}
} // namespace AetherEditor::UI