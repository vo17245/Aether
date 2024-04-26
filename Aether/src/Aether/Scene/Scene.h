#pragma once
#include "Entity.h"
#include <entt.hpp>

#include <filesystem>
#include <unordered_map>
namespace Aether {
class SceneSerializer;

class Scene {

  friend class SceneSerializer;
  friend class Entity;

public:
  Entity CreateEntity();
  void DeleteEntity(Entity &entity);
  template <typename... Components> auto GetAllEntitiesWith() {
    return m_Registry.view<Components...>();
  }

private:
  entt::registry m_Registry;
};
} // namespace Aether
