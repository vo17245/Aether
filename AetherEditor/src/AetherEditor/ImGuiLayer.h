#pragma once
#include "Window/Window.h"
#include <AetherEditor/Window/MainWindow.h>
#include <AetherEditor/Page/MainPage.h>
#include <AetherEditor/Page/EntryPage.h>
#include <AetherEditor/Page/CreateProjectPage.h>
using namespace Aether;
namespace AetherEditor
{


class ImGuiLayer : public Layer
{
public:
    ~ImGuiLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        m_MainWindow.SetOsWindow(window);
        m_MainWindow.PushPage(CreateScope<UI::EntryPage>());
        m_MainWindow.PushPage(CreateScope<UI::MainPage>());
        m_MainWindow.PushPage(CreateScope<UI::CreateProjectPage>());
        m_MainWindow.SetCurrentPage("EntryPage");
    }
    virtual void OnUpdate(float sec) override
    {
        m_MainWindow.OnUpdate(sec);
    }
    virtual bool NeedRebuildRenderGraph() override
    {
        return false;
    }
   
    virtual void OnImGuiUpdate() override
    {
        m_MainWindow.Draw();
    }

private:
    Aether::Window* m_Window = nullptr;
    UI::MainWindow m_MainWindow;
    
};
}