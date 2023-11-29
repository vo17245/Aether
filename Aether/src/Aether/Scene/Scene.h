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
		void DeleteEntity(Entity& entity);
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}
		inline std::function<void(Entity&)>& GetOnCreate() { return m_OnCreate; }
		inline std::function<void(Entity&)>& GetOnDelete() { return m_OnDelete; }
	private:
		entt::registry m_Registry;
		std::function<void(Entity&)> m_OnCreate = [](Entity&) {};
		std::function<void(Entity&)> m_OnDelete = [](Entity&) {};
		
	};
}
