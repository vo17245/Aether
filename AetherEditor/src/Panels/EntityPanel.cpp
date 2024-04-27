#include "EntityPanel.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Scene/Component.h"
#include "Core/MainScene.h"
#include <stdint.h>
#include "Aether/Scene.h"
namespace Aether
{
    namespace Editor
    {
        using AllComponent = Aether::Reflection::ComponentTypeGroup<TagComponent,
            IDComponent, TransformComponent,SkyboxComponent>;
        static std::string GetBriefName(const std::string& fullName)
        {
            std::string res;
            //去掉命名空间
            auto pos = fullName.find_last_of("::");
            if (pos == std::string::npos)
            {
                return fullName;
            }
            res=fullName.substr(pos + 1);
            //去掉component 后缀
            pos = res.find("Component");
            if (pos == std::string::npos)
            {
                return res;
            }
            res = res.substr(0, pos);
            return res;
        }
        struct ComponentHandler
        {
            ComponentHandler(Entity& _e)
                :e(_e) {}
            Entity& e;
            template<typename T>
            void operator()(Reflection::ComponentType<T>)
            {
                if (e.HasComponent<T>())
                {
                    if (ImGui::TreeNode(
                        GetBriefName(
                            std::string(Aether::Reflection::GetComponentTypeString<T>())
                        ).c_str()
                    ))
                    {
        
                        ImGui::TreePop();
                    }
                }
                
            }
        };  
        void EntityPanel::OnImGuiRender() 
        {
            auto& mainScene=MainScene::GetInstance();
                
            ImGui::Begin("Entity Panel");
            if (mainScene.HasEntitySelected())
            {
                auto& entity = mainScene.GetEntitySelected();
                Reflection::ForEachComponentType(AllComponent{},ComponentHandler(entity));
                //auto& entity = mainScene.GetEntitySelected();
                //auto& ic = entity.GetComponent<IDComponent>();
                //ImGui::Text(fmt::format("Entity ID: {}", uint64_t(ic.id)).c_str());
                ////tag?
                //if (entity.HasComponent<TagComponent>())
                //{
                //    auto& tc = entity.GetComponent<TagComponent>();
                //    ImGui::Text(fmt::format("Tag: {}", tc.tag).c_str());
                //    if(ImGui::Button("Edit Tag"))
                //    {
                //        auto* msg = new Message::EditTagBegin();
                //        MessageBus::GetSingleton/().Publish<Message::EditTagBegin>/(msg);
                //    }
                //}
                ////mesh?
                //if (entity.HasComponent<MeshComponent>())
                //{
                //    auto& mc = entity.GetComponent<MeshComponent>();
                //    std::string meshName;
                //    if (mc.filePath)
                //    {
                //        meshName = mc.filePath.value();
                //    }
                //    else 
                //    {
                //        if (mc.model)
                //        {
                //            meshName = "no name mesh";
                //        }
                //        else
                //        {
                //            meshName = "empty";
                //        }
                //    }
                //    ImGui::Text(fmt::format("Mesh: {}", meshName).c_str());
                //}
                ////lua script?
                //if (entity.HasComponent<LuaScriptComponent>())
                //{
                //    auto& lsc = entity.GetComponent<LuaScriptComponent>();
                //    ImGui::Text(fmt::format("Lua Script: {}", lsc.path).c_str());
                //}
                ////add component ?
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