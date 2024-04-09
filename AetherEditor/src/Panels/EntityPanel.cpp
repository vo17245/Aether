#include "EntityPanel.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Scene/Component.h"
#include "Core/MainScene.h"
#include <stdint.h>

namespace Aether
{
    namespace Editor
    {
        void EntityPanel::OnImGuiRender() 
        {
            auto& mainScene=MainScene::GetInstance();
                
            ImGui::Begin("Entity Panel");
            if (mainScene.HasEntitySelected())
            {
                auto& entity = mainScene.GetEntitySelected();
                auto& ic = entity.GetComponent<IDComponent>();
                ImGui::Text(fmt::format("Entity ID: {}", uint64_t(ic.id)).c_str());
                //tag?
                if (entity.HasComponent<TagComponent>())
                {
                    auto& tc = entity.GetComponent<TagComponent>();
                    ImGui::Text(fmt::format("Tag: {}", tc.tag).c_str());
                    if(ImGui::Button("Edit Tag"))
                    {
                        auto* msg = new Message::EditTagBegin();
                        MessageBus::GetSingleton().Publish<Message::EditTagBegin>(msg);
                    }
                }
                bool isClicked = ImGui::Button("Add Component");
                if (isClicked)
                {
                    auto* msg = new Message::AddComponent();
                    MessageBus::GetSingleton().Publish<Message::AddComponent>(nullptr);
                }

            }
            else
            {
                ImGui::Text("no entity selected");
            }
            ImGui::End();
        }
    }
}