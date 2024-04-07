#include "SceneHierarchyPanel.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Scene/Component.h"
#include "Core/MainScene.h"
namespace Aether
{
    namespace Editor
    {
        void SceneHierarchyPanel::OnImGuiRender() 
        {
            MainScene& scene=MainScene::GetInstance();
            auto view=scene.GetScene().GetAllEntitiesWith<IDComponent>();
            ImGui::Begin("Scene Hierarchy");
            for(auto [entity,ic]:view.each())
            {
                Entity e={entity,&scene.GetScene()};
                std::string displayName=std::to_string(uint64_t(ic.id));
                //tag?
                if(e.HasComponent<TagComponent>())
                {
                    auto& tc=e.GetComponent<TagComponent>();
                    displayName=tc.tag;
                }
                //is selected?
                bool is_selected = false;
                if (scene.HasEntitySelected())
                {
                    auto& entitySelected = scene.GetEntitySelected();
                    if (entitySelected == e)
                    {
                        is_selected = true;
                    }
                }
                //button color
                ImVec4 buttonColor(0.3f, 0.3f, 0.3f, 1.f);
                if (is_selected)
                {
                    buttonColor = ImVec4(1.f, 0.8f, 0.f, 1.f);
                }
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor); 
                ImGui::Button(displayName.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::End();

        }
        
        
        
    }
}
