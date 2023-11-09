#include "Scene.h"

namespace Aether
{
	Entity Scene::CreateEntity()
	{
		Entity entity = { m_Registry.create(), this };
		return entity;
	}
	
}

