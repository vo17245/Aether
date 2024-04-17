#include "Aether/Scene/LuaScriptSystem.h"
#include "Aether/Scene/Component.h"
#include "Aether/Script/Lua.h"
#include "Aether/Script/Lua/lua.h"



    AETHER_LUA_C_FUNC(log_info)
    {
        auto lop=Aether::LuaStateOperator(L);
        auto msg=lop.GetArg<std::string>(1);
        Aether::Log::Info("{}",msg);
        return 0;
    }
    AETHER_LUA_C_FUNC(log_error)
    {
        auto lop=Aether::LuaStateOperator(L);
        auto msg=lop.GetArg<std::string>(1);
        Aether::Log::Error("{}",msg);
        return 0;
    }
    
namespace Aether
{
    void LuaScriptSystem::InitEnv()
    {
        auto lop=LuaStateOperator(L);
        lop.RegisterFunction("log_info",AETHER_LUA_C_FUNC_NAME(log_info));
        lop.RegisterFunction("log_error", AETHER_LUA_C_FUNC_NAME(log_error));
    }
    void LuaScriptSystem::OnUpdate(float sec)
    {
        auto view=m_Scene->GetAllEntitiesWith<IDComponent,LuaScriptComponent>();
        for (const auto& [entity,ic,lsc] : view.each())
        {
            auto lop=LuaStateOperator(L);
            auto err=lop.DoString(lsc.script);
            if(err)
            {
                Aether::Log::Error("entity id: {} ,lua script error:{}",ic.id,err.value());
            }
        }
    }
}