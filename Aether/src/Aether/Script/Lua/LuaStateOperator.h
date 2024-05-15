#pragma once
#include "VM/lua.h"
#include "VM/lauxlib.h"
#include "VM/lualib.h"
#include <optional>
#include <string>

namespace Aether {

using LuaUserdata = void*;
class LuaStateOperator
{
public:
    // operator not take the ownership of the lua_State
    LuaStateOperator(lua_State* L)
        : m_L(L)
    {}
    void RegisterFunction(const std::string& name, lua_CFunction func)
    {
        lua_register(m_L, name.c_str(), func);
    }
    void InitLibs()
    {
        luaL_openlibs(m_L);
    }
    int GetArgsCnt()
    {
        return lua_gettop(m_L);
    }
    template <typename T>
    T GetArg(int i)
    {
        static_assert(sizeof(T) == 0, "not support this type");
    }
    template <>
    int GetArg<int>(int i)
    {
        return luaL_checkinteger(m_L, i);
    }
    template <>
    float GetArg<float>(int i)
    {
        return luaL_checknumber(m_L, i);
    }
    template <>
    std::string GetArg<std::string>(int i)
    {
        return std::string(luaL_checkstring(m_L, i));
    }
    template <>
    bool GetArg<bool>(int i)
    {
        return lua_toboolean(m_L, i);
    }
    template <>
    LuaUserdata GetArg<LuaUserdata>(int i)
    {
        return lua_touserdata(m_L, i);
    }
    template <typename T>
    void Push(T val)
    {
        static_assert(sizeof(T) == 0, "not support this type");
    }
    template <>
    void Push<int>(int val)
    {
        lua_pushinteger(m_L, val);
    }
    template <>
    void Push<float>(float val)
    {
        lua_pushnumber(m_L, val);
    }
    template <>
    void Push<std::string>(std::string val)
    {
        lua_pushstring(m_L, val.c_str());
    }
    template <>
    void Push<bool>(bool val)
    {
        lua_pushboolean(m_L, val);
    }
    template <>
    void Push<LuaUserdata>(LuaUserdata val)
    {
        lua_pushlightuserdata(m_L, val);
    }
    std::optional<std::string> DoFile(const std::string& path)
    {
        int retLoad = luaL_dofile(m_L, path.c_str());
        if (retLoad != 0)
        {
            std::string err = lua_tostring(m_L, -1);
            lua_pop(m_L, 1);
            return err;
        }
        return std::nullopt;
    }
    void Close()
    {
        lua_close(m_L);
    }
    std::optional<std::string> DoString(const std::string& str)
    {
        int retLoad = luaL_dostring(m_L, str.c_str());
        if (retLoad != 0)
        {
            std::string err = lua_tostring(m_L, -1);
            lua_pop(m_L, 1);
            return err;
        }
        return std::nullopt;
    }

    void SetGlobal(const std::string& name, int v)
    {
        lua_pushinteger(m_L, v);
        lua_setglobal(m_L, name.c_str());
    }
    void SetGlobal(const std::string& name, float v)
    {
        lua_pushnumber(m_L, v);
        lua_setglobal(m_L, name.c_str());
    }
    void SetGlobal(const std::string& name, const std::string& v)
    {
        lua_pushstring(m_L, v.c_str());
        lua_setglobal(m_L, name.c_str());
    }
    void SetGlobal(const std::string& name, bool v)
    {
        lua_pushboolean(m_L, v);
        lua_setglobal(m_L, name.c_str());
    }
    void SetGlobal(const std::string& name, LuaUserdata v)
    {
        lua_pushlightuserdata(m_L, v);
        lua_setglobal(m_L, name.c_str());
    }
    void BeginTable()
    {
        lua_newtable(m_L);
    }

    void EndTableAsGlobal(const std::string& name)
    {
        lua_setglobal(m_L, name.c_str());
    }
    void TableSet(const std::string& name, int v)
    {
        lua_pushinteger(m_L, v);
        lua_setfield(m_L, -2, name.c_str());
    }
    void TableSet(const std::string& name, double v)
    {
        lua_pushnumber(m_L, v);
        lua_setfield(m_L, -2, name.c_str());
    }
    void TableSet(int index, int v)
    {
        lua_pushinteger(m_L, v);
        lua_rawseti(m_L, -2, index);
    }
    void TableSet(int index, double v)
    {
        lua_pushnumber(m_L, v);
        lua_rawseti(m_L, -2, index);
    }
    void TableSet(int index, const std::string& v)
    {
        lua_pushstring(m_L, v.c_str());
        lua_rawseti(m_L, -2, index);
    }
    void TableSet(const std::string& name, const std::string& v)
    {
        lua_pushstring(m_L, v.c_str());
        lua_setfield(m_L, -2, name.c_str());
    }
    void EndTableAsField(const std::string& name)
    {
        lua_setfield(m_L, -2, name.c_str());
    }

private:
    lua_State* m_L;
};

} // namespace Aether