#pragma once
#include "Panel.h"
namespace AetherEditor::UI
{

class EntryPanel:public Panel
{
public:
    void OnImGuiUpdate()override
    {
        
    }
    void SetOnCreateNewProjectCallback(const std::function<void()>& callback)
    {
        m_OnCreateNewProject = callback;
    }
    void SetOnOpenExistingProjectCallback(const std::function<void()>& callback)
    {
        m_OnOpenExistingProject = callback;
    }
private:
    std::function<void()> m_OnCreateNewProject;
    std::function<void()> m_OnOpenExistingProject;
};
}