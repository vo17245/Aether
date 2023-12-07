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
		inline const std::function<void(Entity&)>& GetEntityCreateHandler()const 
		{ return m_EntityCreateHandler; }
		inline const std::function<void(Entity&)>& GetEntityDestoryHandler()const 
		{ return m_EntityDestoryHandler; }
		inline void SetEntityCreateHandler(const std::function<void(Entity&)>& handler)
		{
			m_EntityCreateHandler=handler;
		}
		inline void SetEntityDestoryHandler(const std::function<void(Entity&)>& handler)
		{
			m_EntityDestoryHandler=handler;
		}
	private:
		entt::registry m_Registry;
		std::function<void(Entity&)> m_EntityCreateHandler = [](Entity&) {};
		std::function<void(Entity&)> m_EntityDestoryHandler = [](Entity&) {};
		
	};
}
