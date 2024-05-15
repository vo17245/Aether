#include "LuaCoreRuntime.h"
#include "VM/lualib.h"
#include "LuaStateOperator.h"
#include "Aether/Core/Log.h"
namespace Aether {
AETHER_LUA_C_FUNC(log_info)
{
    auto lop = Aether::LuaStateOperator(L);
    auto msg = lop.GetArg<std::string>(1);
    Aether::Log::Info("{}", msg);
    return 0;
}
AETHER_LUA_C_FUNC(log_error)
{
    auto lop = Aether::LuaStateOperator(L);
    auto msg = lop.GetArg<std::string>(1);
    Aether::Log::Error("{}", msg);
    return 0;
}
AETHER_LUA_C_FUNC(log_debug)
{
    auto lop = Aether::LuaStateOperator(L);
    auto msg = lop.GetArg<std::string>(1);
    Aether::Log::Debug("{}", msg);
    return 0;
}
void LuaCoreRuntime::Enable(lua_State* L)
{
    // lua lib
    luaL_openlibs(L);
    LuaStateOperator lop(L);
    // aether core
    // log
    lop.RegisterFunction("log_info", AETHER_LUA_C_FUNC_NAME(log_info));
    lop.RegisterFunction("log_error", AETHER_LUA_C_FUNC_NAME(log_error));
    lop.RegisterFunction("log_debug", AETHER_LUA_C_FUNC_NAME(log_debug));
}

} // namespace Aether