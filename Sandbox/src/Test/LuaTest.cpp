#include "LuaTest.h"
#include "Aether/Script/Lua/lauxlib.h"
#include "Test/TestRegister.h"
#include "Aether/Core/Assert.h"
namespace Aether
{
	namespace Test
	{
		REGISTER_TEST(LuaTest);
		LuaTest::LuaTest()
		{
			L = luaL_newstate();
			luaL_openlibs(L);
			int retLoad = luaL_dofile(L, "D:/test.lua");
			AETHER_ASSERT(retLoad == 0 && "failed to load lua src");
			retLoad=luaL_dofile(L, "D:/test.lua");
			AETHER_ASSERT(retLoad==0&&"failed to load lua src");
			lua_getglobal(L, "Hello");
			int retCall = lua_pcall(L, 0, 0, 0);
			if (retCall != 0)
			{
				std::string err = lua_tostring(L, -1);
				Log::Error("{}", err);
				AETHER_ASSERT(false && "failed to call func");
			}
			lua_close(L);
		}
		LuaTest::~LuaTest()
		{

		}
	}
}