#pragma once

#include "Aether/Script/Lua.h"
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