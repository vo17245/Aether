#include "SceneHierachyPanel.h"
#include "Aether/ImGui/ImGui"
#include "Aether/Scene/Component.h"
#include "panel/SceneHierachyPanel.h"

namespace Aether
{
    void SceneHierachyPanel::OnImGuiRender()
    {
        if(!m_Scene)
		{
			return;
		}
	    ImGui::Begin("SceneHierachy",&m_Open);
	    auto view = m_Scene->GetAllEntitiesWith<IDComponent>();
	    for (const auto& [entity, ic] : view.each())
	    {
	    	Entity e = { entity,m_Scene.get()};
	    	std::string displayName;
	    	if(e.HasComponent<TagComponent>())
	    	{
	    		auto& tgc=e.GetComponent<TagComponent>();
	    		displayName=tgc.tag;
	    	}
	    	else 
	    	{
	    		displayName=fmt::format("[{}]",std::to_string(uint64_t(ic.id)));
    
	    	}
	    	ImGui::PushID(ic.id);
	    	if (ImGui::TreeNode(displayName.c_str()))
	    	{
	    		m_SelectedEntity=e;
	    		if(e.HasComponent<TagComponent>())
	    		{
	    			auto& entityTextInputBuf = m_EntityTextInputBuffer[ic.id];
	    			auto& tgc = e.GetComponent<TagComponent>();
	    			if (ImGui::TreeNode("Tag"))
	    			{
	    				ImGui::Text("value: %s",tgc.tag.c_str());
	    				ImGui::Text("input buffer: %s", (const char*)entityTextInputBuf.data());
	    				//ImGui::Text("edit buffer: %s", entity_buf.c_str());
	    				ImGui::InputText("tag", (char*)entityTextInputBuf.data(),
	    					entityTextInputBuf.size());
	    				if (ImGui::Button("apply edit"))
	    				{
	    					tgc.tag = (const char*)entityTextInputBuf.data();
	    				}
	    				ImGui::TreePop();
	    			}
	    		}
	    		if (e.HasComponent<TransformComponent>())
	    		{
	    			if (ImGui::TreeNode("Transform"))
	    			{
	    				auto& tc = e.GetComponent<TransformComponent>();
	    				ImGui::InputFloat3("position", tc.position.data());
	    				ImGui::TreePop();
	    			}
	    		}
	    		
	    		if (e.HasComponent<PointLightComponent>())
	    		{
	    			if (ImGui::TreeNode("PointLight"))
	    			{
	    				auto& lc = e.GetComponent<PointLightComponent>();
	    				ImGui::InputFloat3("color", lc.light.GetColor().data());
	    				ImGui::InputFloat3("position", lc.light.GetPosition().data());
	    				ImGui::TreePop();
	    			}
	    		}
				if (e.HasComponent<VisualComponent>())
	    		{
	    			if (ImGui::TreeNode("VisualComponent"))
	    			{
	    				//auto& vc = e.GetComponent<VisualComponent>();
						//vc.model->GetPath();
	    				
	    				ImGui::TreePop();
	    			}
	    		}
	    		ImGui::TreePop();
	    	}
	    	ImGui::PopID();
	    }
	    if (m_SelectedEntity)
	    {
	    	auto& e = m_SelectedEntity.value();
	    	std::string displayName;
	    	if (e.HasComponent<TagComponent>())
	    	{
	    		displayName = e.GetComponent<TagComponent>().tag;
	    	}
	    	else
	    	{
	    		displayName = std::to_string(e.GetComponent<IDComponent>().id);
	    	}
	    	ImGui::Text("selected: %s", displayName.c_str());
	    	static int componentIndex;
	    	ImGui::Combo("component",&componentIndex , "Tag\0Transform\0Visual\0PointLight\0");
	    	if (ImGui::Button("add component"))
	    	{
	    		switch (componentIndex) 
	    		{
	    		case 0://tag
	    			m_SelectedEntity->AddOrReplaceComponent<TagComponent>();
	    			break;
	    		case 1://tranform
	    			m_SelectedEntity->AddOrReplaceComponent<TransformComponent>();
	    			break;
	    		case 2://visual
					m_SelectedEntity->AddOrReplaceComponent<VisualComponent>();
	    			break;
				case 3://point light
					m_SelectedEntity->AddOrReplaceComponent<PointLightComponent>(Vec3(1,1,1),Vec3(0,0,0));
	    			break;
	    		}
	    	}
	    	if (ImGui::Button("remove component"))
	    	{
	    		switch (componentIndex) 
	    		{
	    		case 0://tag
					if(m_SelectedEntity->HasComponent<TagComponent>())
					{
						m_SelectedEntity->RemoveComponent<TagComponent>();
					}
	    			break;
	    		case 1://tranform
					if(m_SelectedEntity->HasComponent<TransformComponent>())
					{
	    				m_SelectedEntity->RemoveComponent<TransformComponent>();
					}
	    			break;
	    		case 2://visual
					if(m_SelectedEntity->HasComponent<VisualComponent>())
					{
	    				m_SelectedEntity->RemoveComponent<VisualComponent>();
					}
	    			break;
				case 3://point light
					if(m_SelectedEntity->HasComponent<PointLightComponent>())
					{
	    				m_SelectedEntity->RemoveComponent<PointLightComponent>();
					}
	    			break;
	    		}
	    	}
	    }
	    ImGui::End();
    }
	void SceneHierachyPanel::AttachScene(Ref<Scene>& scene)
	{
		m_Scene=scene;
		auto view = m_Scene->GetAllEntitiesWith<IDComponent>();
		for (const auto& [entity, ic] : view.each())
		{
			Entity e = { entity, m_Scene.get() };
			OnEntityCreate(e);
		}
	}
	void SceneHierachyPanel::OnEntityCreate(Entity& entity)
	{
		auto& ic=entity.GetComponent<IDComponent>();
		m_EntityTextInputBuffer[ic.id]=std::vector<std::byte>(128);
	}
    void SceneHierachyPanel::OnEntityDestory(Entity& entity)
	{
		auto& ic=entity.GetComponent<IDComponent>();
		auto iter= m_EntityTextInputBuffer.find(ic.id);
		m_EntityTextInputBuffer.erase(iter);
	}
}