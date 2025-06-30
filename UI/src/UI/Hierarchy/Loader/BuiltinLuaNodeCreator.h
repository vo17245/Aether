#pragma once
#include "NodeCreator.h"
#include "UI/Render/Quad.h"
#include "sol/table.hpp"
#include "sol/types.hpp"
#include <expected>

namespace Aether::UI::Lua
{
    struct GridNodeCreator 
    {
        std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const sol::table& luaNode)
        {
            auto* node=hierarchy.CreateNode();
            auto lua_width=luaNode["width"];
            float width=100;
            if(lua_width.get_type()==sol::type::number)
            {
                width=lua_width;
            }
            auto lua_height=luaNode["height"];
            float height=100;
            if(lua_height.get_type()==sol::type::number)
            {
                height=lua_height;
            }
            auto& base = hierarchy.GetComponent<BaseComponent>(node);
            base.size = Vec2f(width, height);
            return node;
        }
    };
    struct QuadNodeCreator 
    {
        std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const sol::table& luaNode)
        {
            auto* node=hierarchy.CreateNode();
            auto lua_width=luaNode["width"];
            float width=100;
            if(lua_width.get_type()==sol::type::number)
            {
                width=lua_width;
            }
            auto lua_height=luaNode["height"];
            float height=100;
            if(lua_height.get_type()==sol::type::number)
            {
                height=lua_height;
            }
            auto& base = hierarchy.GetComponent<BaseComponent>(node);
            base.size = Vec2f(width, height);
            auto lua_color=luaNode["color"];
            Vec4f color(1, 1, 1, 1);
            if(lua_color.get_type()==sol::type::table)
            {
                auto lua_colorTable=lua_color.get<sol::table>();
                size_t lua_colorTableSize=lua_colorTable.size();
                if(lua_colorTableSize>=4)
                {
                    color.x()=lua_color[1];
                    color.y()=lua_color[2];
                    color.z()=lua_color[3];
                    color.w()=lua_color[4];
                }
                else 
                {
                    return std::unexpected<std::string>("invalid color: less than 4 element");
                
                }
            }
            QuadDesc desc;
            desc.color=color;
            auto& quadComponent = hierarchy.AddComponent<QuadComponent>(node,desc);
            return node;
        }
    };
    struct TextNodeCreator 
    {
        std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const sol::table& luaNode)
        {
            auto* node=hierarchy.CreateNode();
            auto lua_width=luaNode["width"];
            float width=100;
            if(lua_width.get_type()==sol::type::number)
            {
                width=lua_width;
            }
            auto lua_height=luaNode["height"];
            float height=100;
            if(lua_height.get_type()==sol::type::number)
            {
                height=lua_height;
            }
            float size=36;
            auto lua_size=luaNode["size"];
            if(lua_size.get_type()==sol::type::number)
            {
                size=lua_size;
            }
            auto& base = hierarchy.GetComponent<BaseComponent>(node);
            base.size = Vec2f(width, height);
            auto lua_text=luaNode["text"];
            std::string text;
            if(lua_text.get_type()==sol::type::string)
            {
                text=lua_text;
            }
            else  
            {
                return std::unexpected<std::string>("invalid text: not a string");
            }
            auto& textComponent = hierarchy.AddComponent<TextComponent>(node, text);
            textComponent.worldSize = size;
            

            return node;
        }
    };
}