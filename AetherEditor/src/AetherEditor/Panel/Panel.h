#pragma once
#include <ImGui/ImGui.h>
#include <string>
#include <Core/Core.h>
using namespace Aether;
class Panel
{
public:
    virtual ~Panel()=default;
    virtual std::string& GetTitle()=0;
    virtual void SetVisible(bool visible)=0;
    virtual bool GetVisible()=0;
    virtual void OnImGuiUpdate()=0;
    virtual void SetPosition(float x, float y) =0;
    virtual void SetSize(float width, float height)=0;
protected:
    std::string m_Title = "Panel";
    bool m_Visible = true;
};