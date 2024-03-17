#pragma once
#include "Aether/Script/Lua/lua.h"
#include "Aether/Script/Lua/lualib.h"
#include "Aether/Script/Lua/lauxlib.h"
#include "Test/Test.h"
namespace Aether
{
    namespace Test
    {
        class LuaTest:public Test
        {
        public:
            LuaTest();
            ~LuaTest();
        private:
            lua_State* L;
        };
    }
    
}