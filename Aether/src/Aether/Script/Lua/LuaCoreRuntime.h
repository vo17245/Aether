#pragma once
#include "VM/lua.h"
#include <optional>
#include <string>
#define AETHER_LUA_C_FUNC_NAME(name) AETHER_LUA_C_FUNC_##name
#define AETHER_LUA_C_FUNC(name) extern "C" int AETHER_LUA_C_FUNC_NAME(name)(lua_State * L)
namespace Aether {
class LuaCoreRuntime
{
public:
    static void Enable(lua_State* L);
};

}; // namespace Aether