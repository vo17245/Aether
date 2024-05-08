#pragma once
#include "Aether/Scene.h"
#include "Aether/Script/Lua.h"
#include <stdint.h>
#include <string>
namespace Aether {
class LuaScriptSystem
{
public:
public:
    LuaScriptSystem(Scene* scene) :
        m_Scene(scene)
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        L_Camera = luaL_newstate();
        luaL_openlibs(L_Camera);
        InitEnv();
    }
    void InitEnv();
    void OnUpdate(float sec);

private:
    Scene* m_Scene;
    lua_State* L;
    lua_State* L_Camera;
    void UpdateLuaScript(float sec);
    void UpdateLuaCameraScript(float sec);
    void InitCameraScriptEnv();
    void InitScriptEnv();
};
} // namespace Aether