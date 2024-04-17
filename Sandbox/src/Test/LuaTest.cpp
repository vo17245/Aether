#include "LuaTest.h"
#include "Aether/Script/Lua/lauxlib.h"
#include "Test/TestRegister.h"
#include "Aether/Core/Assert.h"
#include "Aether/Script/Lua.h"
#include <string>
extern "C"
{
	
static int lua_log_info (lua_State *L) {
	auto lop = Aether::LuaStateOperator(L);
	auto msg=lop.GetArg<std::string>(1);
	Aether::Log::Info("{}",msg);
	return 0;
}

}
namespace Aether
{
	namespace Test
	{
		REGISTER_TEST(LuaTest);
		LuaTest::LuaTest()
		{
			L = luaL_newstate();
			auto lop=LuaStateOperator(L);
			lop.InitLibs();
			lop.RegisterFunction("log_info",lua_log_info);

			auto err = lop.DoString(R"(
log_info("hello from lua");
)");
			if(err)
			{
				Aether::Log::Error("{}",err.value());
			}
			lop.Close();
		}
		LuaTest::~LuaTest()
		{

		}
	}
}