#pragma once
#include "../Core/Assert.h"
#include "../Core/UUID.h"
#include "Component.h"
#include "entt.hpp"
#include <assert.h>
#include <entt.hpp>
#include <filesystem>
#include <optional>
namespace Aether {
class Scene;
class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene *scene);
  Entity(const Entity &other) = default;
  template <typename T, typename... Args> T &AddComponent(Args &&...args) {
    AETHER_ASSERT(!HasComponent<T>() && "Entity already has component!");

    T &component =
        GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

    OnComponentAdd(component);
    return component;
  }

  template <typename T, typename... Args>
  T &AddOrReplaceComponent(Args &&...args) {
    T &component = GetRegistry().emplace_or_replace<T>(
        m_EntityHandle, std::forward<Args>(args)...);
    OnComponentAdd(component);
    return component;
  }

  template <typename T> T &GetComponent() {
    AETHER_ASSERT(HasComponent<T>() && "Entity does not have component!");

    return GetRegistry().get<T>(m_EntityHandle);
  }

  template <typename T> bool HasComponent() {

    return GetRegistry().any_of<T>(m_EntityHandle);
  }

  template <typename T> void RemoveComponent() {
    AETHER_ASSERT(HasComponent<T>() && "Entity does not have component!");
    OnComponentRemove<T>();
    GetRegistry().remove<T>(m_EntityHandle);
  }

  operator bool() const { return m_EntityHandle != entt::null; }
  operator entt::entity() const { return m_EntityHandle; }
  operator uint32_t() const { return (uint32_t)m_EntityHandle; }
  bool operator==(const Entity &other) const {
    return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
  }

  bool operator!=(const Entity &other) const { return !(*this == other); }

private:
  entt::entity m_EntityHandle{entt::null};
  Scene *m_Scene = nullptr;
  template <typename T> void OnComponentAdd(T &c) {}
  template <typename T> void OnComponentRemove() {}
  entt::registry &GetRegistry();
};
} // namespace Aether
