#pragma once
#include "Page.h"
class MainPage : public Page
{
public:
    MainPage():Page("MainPage")
    {
    }
    virtual void OnImGuiUpdate() 
    {
        
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* 打开文件逻辑 */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* 保存逻辑 */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { /* 退出逻辑 */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {} // 禁用项
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View"))
            {

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }
};