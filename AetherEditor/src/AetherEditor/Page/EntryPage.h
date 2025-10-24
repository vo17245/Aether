#pragma once
#include "Page.h"
class EntryPage : public Page
{
public:
    EntryPage():Page("EntryPage")
    {
    }
    virtual void OnImGuiUpdate() 
    {
        
        ImGui::Begin("Welcome to Aether Editor!", nullptr, ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove|
        ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("Get Started:");
        if (ImGui::Button("Create New Project"))
        {
            GetPageRouter()->NavigateTo("CreateProjectPage");
        }
        ImGui::SameLine();
        if (ImGui::Button("Open Existing Project"))
        {
            GetPageRouter()->NavigateTo("SelectProjectPage");
        }
        ImGui::End();
    }
};