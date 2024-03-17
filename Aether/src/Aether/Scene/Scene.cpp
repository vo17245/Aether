#include "Scene.h"

namespace Aether
{
	Entity Scene::CreateEntity()
	{
		
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>();
		return entity;
	}
	void Scene::DeleteEntity(Entity& entity)
	{
		m_Registry.destroy(entity);
		
	}
}

