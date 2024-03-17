#include "Entity.h"
#include "Scene.h"
namespace Aether
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}

	entt::registry& Entity::GetRegistry()
	{
		return m_Scene->m_Registry;
	}
	
}
