#pragma once
#include "Aether/Message/MessageBus.h"
#include "UI/UI.h"
#include "Aether/ImGui.h"
#include "Aether/Core.h"
#include "Aether/UI/FileDialog.h"
namespace Aether
{
    namespace Editor
    {
        namespace UI
        {
            class FileSelect:public UI
            {
            public:
                FileSelect() = default;
                ~FileSelect() = default;
                void OnEvent(Event& event) override;
                void OnImGuiRender() 
                {
                    ImVec2 windowPos = ImVec2(
                        ImGui::GetIO().DisplaySize.x * 0.5f,
                        ImGui::GetIO().DisplaySize.y * 0.5f);
                    ImVec2 windowPosPivot = ImVec2(0.5f, 0.5f);
                    ImGui::SetNextWindowPos(
                    windowPos, 
                ImGuiCond_Always, 
                windowPosPivot);
            ImGui::SetNextWindowFocus();
                ImGui::Begin("Select Mesh File");
                ImGui::Text(fmt::format("File Path:{}",m_FilePath).c_str());
                bool openExplorer=ImGui::Button("open file explorer");
                bool save =ImGui::Button("save");
                bool cancel =ImGui::Button("cancel");
                ImGui::End();
                if(openExplorer)
                {
                    
                    m_FilePath=OpenFileDialog();
                    return;
                }
                if(cancel)
                {
                    m_OnEnd(m_FilePath );
                }
                if(save)
                {
                    
                }
                }
                void OnAttach() override;
                void OnDetach() override;
                void SetOnEnd(const std::function<void(const std::string&)>& func)
                {
                    m_OnEnd = func;
                }
            private:
                std::string m_FilePath;
                std::function<void(const std::string&)> m_OnEnd;
            };
        }
    }
   
}