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
        }

        SelectComponentPanel::~SelectComponentPanel()
        {
        }

        void SelectComponentPanel::OnImGuiRender()
        {
            ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
             ImGui::GetIO().DisplaySize.y * 0.5f);
            ImVec2 windowPosPivot = ImVec2(0.5f, 0.5f);
            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, 
            windowPosPivot);
            ImGui::SetNextWindowFocus();
            ImGui::Begin("Select Component");
            bool isClicked=ImGui::Button("Tag");
            if(isClicked)
            {
                auto& messageBus=MessageBus::GetSingleton();
                auto* msg=new Message::ComponentSelected(
                    ::Aether::TypeIdProvider<TagComponent>::ID()
                );
                messageBus.Publish<Message::ComponentSelected>(msg);
                if (!MainScene::GetInstance().GetEntitySelected()
                    .HasComponent<TagComponent>())
                {
                    MainScene::GetInstance().GetEntitySelected()
                        .AddComponent<TagComponent>("empty");
                }
                
            }
            bool addMesh = ImGui::Button("Mesh");
            ImGui::End();
            if (addMesh)
            {

            }
        }
    }
}