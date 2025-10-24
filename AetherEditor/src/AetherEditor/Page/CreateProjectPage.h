#pragma once
#include "Page.h"
struct CreateProjectParams
{
    std::string projectName;
    std::string projectLocation;
};
class CreateProjectPage:public Page
{
public:
    CreateProjectPage():Page("CreateProjectPage")
    {
    }
    void OnImGuiUpdate() override;
private:
    CreateProjectParams m_Params;
    char m_ProjectName[128]{0};
    char m_ProjectLocation[128]{0};
    ImVec2 GetSize();
    ImVec2 GetPos();
};