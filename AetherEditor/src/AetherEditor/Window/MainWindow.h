#pragma once
#include <Imgui/Core/imgui.h>
#include <functional>
#include <Window/Window.h>
#include <AetherEditor/Page/Page.h>
#include <AetherEditor/UIComponent/Notify.h>
using namespace Aether;
namespace AetherEditor::UI
{

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();
    void SetOsWindow(::Aether::Window* window)
    {
        m_OsWindow = window;
    }
    void DrawMainWindowEnd()
    {
        ImGui::End();
    }
   
    
    void DrawMainWindowBegin()
    {
        Vec2i size = m_OsWindow->GetSize();
        // full screen
        ImGui::SetNextWindowSize(ImVec2((float)size.x(), (float)size.y()));
        // no title bar
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav
                                        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize
                                        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar
                                        | ImGuiWindowFlags_NoTitleBar;
        //ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        //ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("MainWindow", nullptr, window_flags);
        //ImGui::PopStyleColor(2);
        //ImGui::PopStyleVar();


        // 给父窗口创建独立 DockSpace

        //ImGuiID dockspace_id = ImGui::GetID("Docking");
        //ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
    }

    void Draw()
    {
        DrawMainWindowBegin();
        auto* currentPage = m_PageRouter.GetCurrentPage();
        if (currentPage)
        {
            currentPage->OnImGuiUpdate();
        }
        Notify::Draw();
        DrawMainWindowEnd();
    }

  
    void OnUpdate(float sec)
    {
        Notify::Update(sec);
    }
    void SetCurrentPage(const std::string& tag)
    {
        m_PageRouter.NavigateTo(tag);
    }
    template<typename T>
    requires std::is_base_of_v<Page, T>
    T* PushPage(Scope<T>&& page)
    {
        page->SetPageRouter(&m_PageRouter);
        return m_PageRouter.PushPage(std::move(page));
    }


private:
    ::Aether::Window* m_OsWindow;
    PageRouter m_PageRouter;
    
};
}