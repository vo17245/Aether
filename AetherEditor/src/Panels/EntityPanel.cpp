#include "EntityPanel.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Scene/Component.h"
#include "Core/MainScene.h"
#include <stdint.h>
#include "Aether/Scene.h"
#include "UI/ImGuiStorage.h"
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
            ////去掉component 后缀
            //pos = res.find("Component");
            //if (pos == std::string::npos)
            //{
            //    return res;
            //}
            res = res.substr(0, pos);
            return res;
        }
        template<typename ComponentT>
        struct FieldView
        {
            const char* name;
            ComponentT& component;
            FieldView(const char* _name, ComponentT& _component):name(_name),component(_component){}
            template<typename T>
            void operator()(T& field)
            {
                AETHER_DEBUG_LOG_ERROR("not implemented field type imgui edit ui {}", typeid(T).name());
            }
            template<>
            void operator()(std::string& field)
            {
                //获得当前id
                ImGuiID id = ImGui::GetID(name);
                //string 存储位置
                char* storage = ImGuiStorage::Get(id);
                if (storage[0]==0)
                {
                    //初始化
                    size_t bytes_to_copy = std::min(ImGuiStorage::MAX_STORAGE_SIZE - 1,
                        field.size());
                    memcpy(storage + 1, field.c_str(), bytes_to_copy);
                    storage[0] = 1;
                }
                bool ret=ImGui::InputText(name, storage+1, ImGuiStorage::MAX_STORAGE_SIZE-1);
                if (ret)
                {
                    Reflection::Meta<ComponentT>::Set(component, name, std::string(storage));

                }
            }
            template<>
            void operator()(UUID& uuid)
            {
                ImGui::Text(fmt::format("{}: {}",name,size_t(uuid)).c_str());
            }
            template<>
            void operator()(Mat4& mat)
            {

            }
            template<>
            void operator()(Mat3& mat)
            {

            }
            
        };
        template<typename ComponentT>
        struct FieldHandler
		{
            ComponentT& component;
            FieldHandler(ComponentT& _component) :component(_component) {}
			void operator()(const char* name, const char* type, const char* comment,
				Reflection::CoreComponentFieldVariant& value)
			{
                std::visit(FieldView(name,component),value );
			}
		};
        
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
                    auto& c = e.GetComponent<T>();
                    if (ImGui::TreeNode(
                        GetBriefName(
                            std::string(Aether::Reflection::GetComponentTypeString<T>())
                        ).c_str()
                    ))
                    {
                        Aether::Reflection::ForEachField(c, FieldHandler<T>(c));
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
                auto& ic = entity.GetComponent<IDComponent>();
                ImGui::PushID(ic.id);
                auto handler=ComponentHandler(entity);
                Reflection::ForEachComponentType(AllComponent{},handler);
               
                if (ImGui::Button("Add Component"))
                {
                    auto* msg = new Message::SelectComponentBegin();
                    MessageBus::GetSingleton()
                    .Publish<Message::SelectComponentBegin>(nullptr);
                }
                ImGui::PopID();
            }
            else
            {
                ImGui::Text("no entity selected");
            }
            
            ImGui::End();
        }
    }
}