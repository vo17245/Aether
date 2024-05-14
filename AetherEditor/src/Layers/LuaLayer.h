#pragma once

#include "Aether/IO.h"
#include "Aether/Core.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Script/Lua.h"
#include "Aether/Script/Lua/lua.h"
#include <memory>
namespace Aether {
namespace Editor {
class LuaLayer : public Layer
{
public:
    LuaLayer(const std::string& xmlPath, const std::string& luaScriptPath)
        : m_XmlPath(xmlPath), m_LuaScriptPath(luaScriptPath)
    {}

    virtual ~LuaLayer() = default;
    void OnAttach() override
    {
        m_LuaState = luaL_newstate();
        // load lua script
        LuaStateOperator lop(m_LuaState);
        lop.InitLibs();
        auto doFileErr = lop.DoFile(m_LuaScriptPath);
        if (doFileErr)
        {
            Log::Error("Failed to exec lua script: {},error: {}", m_LuaScriptPath,doFileErr.value());
            return;
        }
        // load xml
        auto xmlRes = XML::LoadFile(m_XmlPath);
        if (!xmlRes)
        {
            Log::Error("Failed to load xml: {}", m_XmlPath);
            return;
        }
        m_Xml = std::make_unique<XML>(std::move(xmlRes.value()));
    }
    void OnImGuiRender() override
    {
        RenderXML();
    }
    void RenderXML()
    {
        if (!m_Xml)
        {
            AETHER_DEBUG_LOG_ERROR("xml is null");
            return;
        }
        for (auto e : XML::EachChildren(m_Xml->GetRoot()))
        {
            auto name = e.GetName();
            if (name == "window")
            {
                RenderWindow(e);
            }
        }
    }
    void RenderWindow(XML::Element& e)
    {
        auto title = e.GetAttribute("title");
        if (!title)
        {
            AETHER_DEBUG_LOG_ERROR("window title is null");
            return;
        }
        ImGui::Begin(title.value().c_str());
        ImGui::End();
    }
    void OnDetach() override
    {
        if (m_LuaState)
        {
            lua_close(m_LuaState);
            m_LuaState = nullptr;
        }
    }

private:
    std::string m_XmlPath;
    std::string m_LuaScriptPath;
    lua_State* m_LuaState = nullptr;
    std::unique_ptr<XML> m_Xml = nullptr;
};
}
} // namespace Aether::Editor