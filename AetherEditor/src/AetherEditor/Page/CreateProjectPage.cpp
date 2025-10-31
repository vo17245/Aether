#include "CreateProjectPage.h"
#include <UI/FileDialog.h>
#include <Project/Project.h>
#include <Filesystem/Filesystem.h>
#include <AetherEditor/Global/GlobalProject.h>
namespace AetherEditor::UI
{
/**
 * @return std::optional<std::string> Returns std::nullopt on success, or an error message on failure.
*/
static std::expected<Scope<Aether::Project::Project>, std::string> CreateProject(const CreateProjectParams& params)
{
    const auto& name=params.projectName;
    const auto& location=params.projectLocation;
    const std::string projectDir=location + "/" + name;
    const std::string contentDir=projectDir + "/Content";
    const std::string projectFilePath=projectDir + "/" + name + ".aetherproj";
    if(Aether::Filesystem::Exists(projectDir))
    {
        return std::unexpected<std::string>("Project directory already exists: " + projectDir);
    }
    auto project=CreateScope<Aether::Project::Project>();
    project->SetProjectFilePath(projectFilePath);
    project->SetProjectName(name);
    project->SetContentDirPath(contentDir);
    auto createDirResult=Aether::Filesystem::CreateDirectory(projectDir);
    if(!createDirResult)
    {
        return std::unexpected<std::string>("Failed to create project directory: " + projectDir);
    }
    auto createContentDirResult=Aether::Filesystem::CreateDirectory(contentDir);
    if(!createContentDirResult)
    {
        return std::unexpected<std::string>("Failed to create content directory: " + contentDir);
    }
    auto saveResult=project->Save();
    if(saveResult)
    {
        return std::unexpected<std::string>("Failed to save project file: " + *saveResult);
    }
    return project;
}

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
        auto dirEx=Aether::UI::SyncSelectDirectroy();
        if(dirEx)
        {
            memcpy(m_ProjectLocation, dirEx->c_str(), dirEx->size()+1);
        }
        else
        {
            LogE("failed to select directory: {}", dirEx.error());
        }
    }
    ImGui::Separator();
    if (ImGui::Button("Create"))
    {
        m_Params.projectName = m_ProjectName;
        m_Params.projectLocation = m_ProjectLocation;
        auto result = CreateProject(m_Params);
        if (!result)
        {
            LogE("failed to create project: {}", result.error());
        }
        else
        {
            GlobalProject::SetProject(std::move(result.value()));
            LogI("Project created successfully: {}", m_Params.projectName);
            GetPageRouter()->NavigateTo("MainPage");
        }
    }
    ImGui::End();
}
ImVec2 CreateProjectPage::GetSize()
{
    return ImVec2(600, 400);
}
ImVec2 CreateProjectPage::GetPos()
{
    return ImVec2(0, 0);
}
}