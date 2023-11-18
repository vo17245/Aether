#pragma once
#include <entt.hpp>
#include "Entity.h"

#include <filesystem>
#include <unordered_map>
namespace Aether
{
	class SceneSerializer;
	class Scene
	{
		friend class SceneSerializer;
		friend class Entity;
	public:
		Entity CreateEntity();
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}
	private:
		entt::registry m_Registry;

	};
}
