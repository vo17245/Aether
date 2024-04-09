#pragma once
#include "Aether/ImGui/imgui.h"
#include "Aether/Message/Message.h"
#include "Aether/Message/SubscribeReclaimer.h"
#include "Panel.h"
#include "Aether/Message.h"
#include "Message/EditorMessage.h"
#include "Aether/UI/FileDialog.h"
#include "Core/MainScene.h"
namespace Aether
{
    namespace Editor
    {
        class MeshFileSelectPanel:public Panel
        {
        public:
            MeshFileSelectPanel() 
            {
                m_SubscribeReclaimer
                .Subscribe<::Aether::Editor::Message::SelectMeshFileBegin>
                ([this](::Aether::Message* msg){
                    m_Show=true;
                });

            };
            ~MeshFileSelectPanel() 
            {
                
            };
            void OnImGuiRender() override
            {
                if(!m_Show)
                {
                    return;
                }
                ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
             ImGui::GetIO().DisplaySize.y * 0.5f);
            ImVec2 windowPosPivot = ImVec2(0.5f, 0.5f);
            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, 
            windowPosPivot);
            ImGui::SetNextWindowFocus();
                ImGui::Begin("Select Mesh File");
                ImGui::Text(fmt::format("File Path:{}",m_FilePath).c_str());
                bool openExplorer=ImGui::Button("open file explorer");
                bool cancel=ImGui::Button("save");
                bool save=ImGui::Button("cancel");
                ImGui::End();
                if(openExplorer)
                {
                    
                    m_FilePath=OpenFileDialog();
                    return;
                }
                if(cancel)
                {
                    m_FilePath="";
                    m_Show=false;
                    return;
                }
                if(save)
                {
                    auto& scene=MainScene::GetInstance();
                    if(scene.HasEntitySelected())
                    {
                        auto& entity=scene.GetEntitySelected();
                        
                    }
                    
                }

            }
        private:
            bool m_Show;
            std::string m_FilePath;
            SubscribeReclaimer m_SubscribeReclaimer;
        };
    }
}