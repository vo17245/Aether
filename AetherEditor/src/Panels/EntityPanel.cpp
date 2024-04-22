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
                //mesh?
                if (entity.HasComponent<MeshComponent>())
                {
                    auto& mc = entity.GetComponent<MeshComponent>();
                    std::string meshName;
                    if (mc.filePath)
                    {
                        meshName = mc.filePath.value();
                    }
                    else 
                    {
                        if (mc.model)
                        {
                            meshName = "no name mesh";
                        }
                        else
                        {
                            meshName = "empty";
                        }
                    }
                    ImGui::Text(fmt::format("Mesh: {}", meshName).c_str());
                }
                //lua script?
                if (entity.HasComponent<LuaScriptComponent>())
                {
                    auto& lsc = entity.GetComponent<LuaScriptComponent>();
                    ImGui::Text(fmt::format("Lua Script: {}", lsc.path).c_str());
                }
                //add component ?
                if (ImGui::Button("Add Component"))
                {
                    auto* msg = new Message::SelectComponentBegin();
                    MessageBus::GetSingleton()
                    .Publish<Message::SelectComponentBegin>(nullptr);
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