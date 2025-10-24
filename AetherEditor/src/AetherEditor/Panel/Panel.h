#pragma once
#include <ImGui/ImGui.h>
#include <string>
#include <Core/Core.h>
using namespace Aether;
class Panel
{
public:
    virtual ~Panel() = default;
    virtual void OnImGuiUpdate()
    {
    }
    virtual void OnSetPosition(float x, float y)
    {
    }
    virtual void OnSetSize(float width, float height)
    {
    }
    virtual void OnSetVisible(bool visible)
    {
    }
public:
    void SetPosition(float x, float y)
    {
        m_Position = {x, y};
        OnSetPosition(x, y);
    }
    void SetSize(float width, float height)
    {
        m_Size = {width, height};
        OnSetSize(width, height);
    }
    void SetVisible(bool visible)
    {
        m_Visible = visible;
        OnSetVisible(visible);
    }
    bool GetVisible()
    {
        return m_Visible;
    }
    std::string& GetTitle()
    {
        return m_Title;
    }
    void SetTitle(const std::string& title)
    {
        m_Title = title;
    }

protected:
    std::string m_Title = "Panel";
    bool m_Visible = true;
    Vec2f m_Position = {0.0f, 0.0f};
    Vec2f m_Size = {100.0f, 100.0f};
};