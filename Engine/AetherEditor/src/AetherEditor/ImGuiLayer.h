#pragma once
#include "Window/Window.h"
#include <AetherEditor/Window/MainWindow.h>
#include <AetherEditor/Page/MainPage.h>
#include <AetherEditor/Page/EntryPage.h>
#include <AetherEditor/Page/CreateProjectPage.h>
#include <AetherEditor/Page/AssetPage.h>
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
        auto* mainPage = m_MainWindow.PushPage(CreateScope<UI::MainPage>());
        m_MainWindow.PushPage(CreateScope<UI::CreateProjectPage>());
        auto* assetPage = m_MainWindow.PushPage(CreateScope<UI::AssetPage>());
        m_MainWindow.SetCurrentPage("EntryPage");
        mainPage->OnAssetClickedEventHandler += [assetPage](const Project::AssetContentNode& asset) {
            assetPage->SetAssetToShow(asset);
        };
        mainPage->OnPageNavigateEventHandler += [this](const std::string& name) {
            m_MainWindow.SetCurrentPage(name);
        };
        assetPage->OnPageNavigateEventHandler += [this](const std::string& name) {
            m_MainWindow.SetCurrentPage(name);
        };
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
} // namespace AetherEditor