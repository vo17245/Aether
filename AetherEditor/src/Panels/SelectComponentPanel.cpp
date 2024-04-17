#include "SelectComponentPanel.h"
#include "Aether/Core/TypeIdProvider.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Message.h"
#include "Aether/Message/Message.h"
#include "Aether/Scene/Component.h"
#include "Message/EditorMessage.h"
#include "Core/MainScene.h"
namespace Aether
{
    namespace Editor
    {
        SelectComponentPanel::SelectComponentPanel()
        {
            m_Reclaimer.Subscribe<Message::SelectComponentBegin>(
                [this](Aether::Message* msg)
            {
                m_Show = true;
            });
        }

        SelectComponentPanel::~SelectComponentPanel()
        {
        }

        void SelectComponentPanel::OnImGuiRender()
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
            ImGui::Begin("Select Component");
            bool addTag=ImGui::Button("Tag");
            bool addMesh = ImGui::Button("Mesh");
            bool addLuaScript=ImGui::Button("LuaScript");
            ImGui::End();
            if (addMesh)
            {
                //add mesh
                if(!MainScene::GetInstance().GetEntitySelected()
                    .HasComponent<MeshComponent>())
                {
                    MainScene::GetInstance().GetEntitySelected()
                        .AddComponent<MeshComponent>();
                }
                //open mesh file select panel
                auto* msg = new Message::SelectMeshFileBegin();
                MessageBus::GetSingleton().Publish<Message::SelectMeshFileBegin>(msg);
                m_Show = false;
            }
            if(addTag)
            {
                //add tag
                if (!MainScene::GetInstance().GetEntitySelected()
                    .HasComponent<TagComponent>())
                {
                    MainScene::GetInstance().GetEntitySelected()
                        .AddComponent<TagComponent>("empty");
                }
                //open tag edit panel
                auto* msg = new Message::EditTagBegin();
                MessageBus::GetSingleton().Publish<Message::EditTagBegin>(msg);
                m_Show = false;
            }
            if(addLuaScript)
            {
                //add lua script
                if(!MainScene::GetInstance().GetEntitySelected()
                    .HasComponent<LuaScriptComponent>())
                {
                    MainScene::GetInstance().GetEntitySelected()
                        .AddComponent<LuaScriptComponent>();
                }
                //open lua script file select panel
                auto* msg = new Message::SelectLuaScriptFileBegin();
                MessageBus::GetSingleton().Publish<Message::SelectLuaScriptFileBegin>(msg);
                m_Show = false;
            }
        }
    }
}