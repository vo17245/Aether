#pragma once

#include "Aether/Core/Math.h"
#include "Aether/IO.h"
#include "Aether/Core.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Script/Lua.h"
#include <memory>

namespace Aether {
namespace Editor {
class UILayer : public Layer
{
public:
    UILayer(const std::string& xmlPath, const std::string& luaScriptPath)
        : m_XmlPath(xmlPath), m_LuaScriptPath(luaScriptPath)
    {}
    // example: "(1,2,3)"
    std::optional<Vec3> StringToVec3(const std::string& s)
    {
    }
    virtual ~UILayer() = default;
    void OnAttach() override
    {
        m_LuaState = luaL_newstate();
        LuaCoreRuntime::Enable(m_LuaState);
        // load lua script
        LuaStateOperator lop(m_LuaState);
        auto doFileErr = lop.DoFile(m_LuaScriptPath);
        if (doFileErr)
        {
            Log::Error("Failed to exec lua script: {},error: {}", m_LuaScriptPath, doFileErr.value());
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
        for (auto e : XML::EachChildren(e))
        {
            auto name = e.GetName();
            if (name == "button")
            {
                RenderButton(e);
            }
        }
        ImGui::End();
    }
    void RenderButton(XML::Element& e)
    {
        auto title = e.GetAttribute("title");
        if (!title)
        {
            AETHER_DEBUG_LOG_ERROR("button title is null");
            return;
        }

        if (ImGui::Button(title.value().c_str()))
        {
            // onClick
            auto onClick = e.GetAttribute("onClick");
            if (onClick)
            {
                LuaStateOperator lop(m_LuaState);
                auto res = lop.DoString(onClick.value());
                if (res)
                {
                    AETHER_DEBUG_LOG_ERROR("Failed to exec lua script: {},error: {}", onClick.value(), res.value());
                }
            }
        }
        // onHover
        if (ImGui::IsItemHovered())
        {
            auto onHover = e.GetAttribute("onHover");
            if (onHover)
            {
                LuaStateOperator lop(m_LuaState);
                auto res = lop.DoString(onHover.value());
                if (res)
                {
                    AETHER_DEBUG_LOG_ERROR("Failed to exec lua script: {},error: {}", onHover.value(), res.value());
                }
            }
        }
        // onActive / onPress
        // in imgui onActive means the button is pressed
        if (ImGui::IsItemActive())
        {
            auto onPress = e.GetAttribute("onPress");
            if (onPress)
            {
                LuaStateOperator lop(m_LuaState);
                auto res = lop.DoString(onPress.value());
                if (res)
                {
                    AETHER_DEBUG_LOG_ERROR("Failed to exec lua script: {},error: {}", onPress.value(), res.value());
                }
            }
        }
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