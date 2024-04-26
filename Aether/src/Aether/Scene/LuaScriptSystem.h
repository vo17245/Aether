#pragma once
#include "Aether/Scene.h"
#include "Aether/Script/Lua.h"
namespace Aether {
class LuaScriptSystem {
public:
  LuaScriptSystem(Scene *scene) : m_Scene(scene) {
    L = luaL_newstate();
    luaL_openlibs(L);
    InitEnv();
  }
  void InitEnv();
  void OnUpdate(float sec);

private:
  Scene *m_Scene;
  lua_State *L;
};
} // namespace Aether