#include "Aether/Scene/LuaScriptSystem.h"
#include "Aether/Scene/Component.h"
#include "Aether/Script/Lua.h"
#include "Aether/Script/Lua/lua.h"
#include <string>

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

AETHER_LUA_C_FUNC(set_camera_position)
{
    auto lop = Aether::LuaStateOperator(L);
    Aether::PerspectiveCameraComponent& pcc =
        *(Aether::PerspectiveCameraComponent*)(lop.GetArg<Aether::LuaUserdata>(1));
    auto x = lop.GetArg<float>(2);
    auto y = lop.GetArg<float>(3);
    auto z = lop.GetArg<float>(4);
    pcc.camera.GetPosition().x() = x;
    pcc.camera.GetPosition().y() = y;
    pcc.camera.GetPosition().z() = z;
    return 0;
}

namespace Aether {
void LuaScriptSystem::InitCameraScriptEnv()
{
    auto lop = LuaStateOperator(L_Camera);
    // log
    lop.RegisterFunction("log_info", AETHER_LUA_C_FUNC_NAME(log_info));
    lop.RegisterFunction("log_error", AETHER_LUA_C_FUNC_NAME(log_error));
    // camera setter
    lop.RegisterFunction("set_camera_position_impl", AETHER_LUA_C_FUNC_NAME(set_camera_position));
    std::optional<std::string> err = lop.DoString(R"(
            function set_camera_position(x,y,z)
                set_camera_position_impl(camera_component_userdata,x,y,z)
            end
    )");

    // storage
    err = lop.DoString(R"(
            local ID="0";
            local ENTITY_STORAGE={}
            function storage()
                if not ENTITY_STORAGE[ID] then
                    ENTITY_STORAGE[ID]={}
                end
                return ENTITY_STORAGE[ID];
            end
        )");
    if (err)
    {
        AETHER_DEBUG_LOG_ERROR("{}", err.value());
    }
    // camera get
}
void LuaScriptSystem::InitScriptEnv()
{
    auto lop = LuaStateOperator(L);
    lop.RegisterFunction("log_info", AETHER_LUA_C_FUNC_NAME(log_info));
    lop.RegisterFunction("log_error", AETHER_LUA_C_FUNC_NAME(log_error));
    auto err = lop.DoString(R"(
            local ID="0";
            local ENTITY_STORAGE={}
            function storage()
                if not ENTITY_STORAGE[ID] then
                    ENTITY_STORAGE[ID]={}
                end
                return ENTITY_STORAGE[ID];
            end
        )");
    if (err)
    {
        AETHER_DEBUG_LOG_ERROR("{}", err.value());
    }
}
void LuaScriptSystem::InitEnv()
{
    InitScriptEnv();
    InitCameraScriptEnv();
}
void LuaScriptSystem::UpdateLuaScript(float sec)
{
    auto lop = LuaStateOperator(L);
    auto view = m_Scene->GetAllEntitiesWith<IDComponent, LuaScriptComponent>();
    for (const auto& [entity, ic, lsc] : view.each())
    {
        // set lua state
        lop.SetGlobal("ID", std::to_string(uint64_t(ic.id)));

        // do file
        auto err = lop.DoString(lsc.script);
        if (err)
        {
            Aether::Log::Error("entity id: {} ,lua script error:{}", ic.id,
                               err.value());
        }
    }
}
void LuaScriptSystem::UpdateLuaCameraScript(float sec)
{
    auto view = m_Scene->GetAllEntitiesWith<IDComponent, LuaCameraScriptComponent, PerspectiveCameraComponent>();
    for (const auto& [entity, ic, lcs, pcc] : view.each())
    {
        auto lop = LuaStateOperator(L_Camera);
        // ID
        lop.SetGlobal("ID", std::to_string(uint64_t(ic.id)));
        // camera data
        lop.BeginTable();
        {
            lop.BeginTable();
            {
                lop.TableSet(1, pcc.camera.GetPosition().x());
                lop.TableSet(2, pcc.camera.GetPosition().y());
                lop.TableSet(3, pcc.camera.GetPosition().z());
            }

            lop.EndTableAsField("position");
        }

        lop.EndTableAsGlobal("camera");
        // camera component pointer
        lop.SetGlobal("camera_component_userdata", &pcc);

        // exec script
        auto err = lop.DoString(lcs.script);
        if (err)
        {
            AETHER_DEBUG_LOG_ERROR("entity id: {} ,lua script error:{}", ic.id,
                                   err.value());
        }
        err = lop.DoString("camera=nil");
        if (err)
        {
            AETHER_DEBUG_LOG_ERROR("entity id: {} ,lua script error:{}", ic.id,
                                   err.value());
        }
    }
}
void LuaScriptSystem::OnUpdate(float sec)
{
    UpdateLuaScript(sec);
    UpdateLuaCameraScript(sec);
}
} // namespace Aether