#pragma once
#include "NodeCreator.h"
#include "UI/Render/Quad.h"
#include "sol/table.hpp"
#include "sol/types.hpp"
#include "../Component/AutoResize.h"
#include <expected>

namespace Aether::UI::Lua
{
enum class NodeSizeType
{
    Pixel,
    Relative,
};
struct NodeSize
{
    NodeSizeType type;
    float size;
};
constexpr inline NodeSize ParseNodeSize(const std::string& s)
{
    NodeSize nodeSize;
    if (s.ends_with("%"))
    {
        nodeSize.type = NodeSizeType::Relative;
        nodeSize.size = std::stof(s.substr(0, s.size() - 1));
    }
    else
    {
        nodeSize.type = NodeSizeType::Pixel;
        nodeSize.size = std::stof(s);
    }
    return nodeSize;
}
struct GridNodeCreator
{
    std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const sol::table& luaNode)
    {
        auto* node = hierarchy.CreateNode();
        auto lua_width = luaNode["width"];
        float width = 100;
        std::optional<float> autoResizeWidth;
        if (lua_width.get_type() == sol::type::number)
        {
            width = lua_width;
        }
        else if (lua_width.get_type() == sol::type::string)
        {
            std::string widthStr = lua_width;
            auto nodeSize = ParseNodeSize(widthStr);
            if (nodeSize.type == NodeSizeType::Pixel)
            {
                width = nodeSize.size;
            }
            else
            {
                // relative size
                autoResizeWidth=nodeSize.size/100.0f;
            }
        }
        auto lua_height = luaNode["height"];
        float height = 100;
        std::optional<float> autoResizeHeight;
        if (lua_height.get_type() == sol::type::number)
        {
            height = lua_height;
        }
        else if (lua_height.get_type()==sol::type::string)
        {
            std::string heightStr= lua_height;
            auto nodeSize = ParseNodeSize(heightStr);
            if (nodeSize.type == NodeSizeType::Pixel)
            {
                height = nodeSize.size;
            }
            else
            {
                // relative size
                autoResizeHeight=nodeSize.size/100.0f;

            }

        }
        auto& base = hierarchy.GetComponent<BaseComponent>(node);
        base.size = Vec2f(width, height);
        if(autoResizeWidth||autoResizeHeight)
        {
            auto& autoResize= hierarchy.AddComponent<AutoResizeComponent>(node);
            if(autoResizeHeight)
            {
                autoResize.height = *autoResizeHeight;
                autoResize.EnableHeight();
            }
            if(autoResizeWidth)
            {
                autoResize.width = *autoResizeWidth;
                autoResize.EnableWidth();
            }
        }
        return node;
    }
};
struct QuadNodeCreator
{
    std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const sol::table& luaNode)
    {
        auto* node = hierarchy.CreateNode();
        auto lua_width = luaNode["width"];
        float width = 100;
        std::optional<float> autoResizeWidth;
        if (lua_width.get_type() == sol::type::number)
        {
            width = lua_width;
        }
        else if (lua_width.get_type() == sol::type::string)
        {
            std::string widthStr = lua_width;
            auto nodeSize = ParseNodeSize(widthStr);
            if (nodeSize.type == NodeSizeType::Pixel)
            {
                width = nodeSize.size;
            }
            else
            {
                // relative size
                autoResizeWidth = nodeSize.size / 100.0f;
            }
        }
        auto lua_height = luaNode["height"];
        float height = 100;
        std::optional<float> autoResizeHeight;
        if (lua_height.get_type() == sol::type::number)
        {
            height = lua_height;
        }
        else if (lua_height.get_type() == sol::type::string)
        {
            std::string heightStr = lua_height;
            auto nodeSize = ParseNodeSize(heightStr);
            if (nodeSize.type == NodeSizeType::Pixel)
            {
                height = nodeSize.size;
            }
            else
            {
                // relative size
                autoResizeHeight = nodeSize.size / 100.0f;
            }
        }

        auto& base = hierarchy.GetComponent<BaseComponent>(node);
        base.size = Vec2f(width, height);
        auto lua_color = luaNode["color"];
        Vec4f color(1, 1, 1, 1);
        if (lua_color.get_type() == sol::type::table)
        {
            auto lua_colorTable = lua_color.get<sol::table>();
            size_t lua_colorTableSize = lua_colorTable.size();
            if (lua_colorTableSize >= 4)
            {
                color.x() = lua_color[1];
                color.y() = lua_color[2];
                color.z() = lua_color[3];
                color.w() = lua_color[4];
            }
            else
            {
                return std::unexpected<std::string>("invalid color: less than 4 element");
            }
        }
        QuadDesc desc;
        desc.color = color;
        auto& quadComponent = hierarchy.AddComponent<QuadComponent>(node, desc);
        if (autoResizeWidth || autoResizeHeight)
        {
            auto& autoResize = hierarchy.AddComponent<AutoResizeComponent>(node);
            if (autoResizeHeight)
            {
                autoResize.height = *autoResizeHeight;
                autoResize.EnableHeight();
            }
            if (autoResizeWidth)
            {
                autoResize.width = *autoResizeWidth;
                autoResize.EnableWidth();
            }
        }
        return node;
    }
};
struct TextNodeCreator
{
    std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const sol::table& luaNode)
    {
        auto* node = hierarchy.CreateNode();
        auto lua_width = luaNode["width"];
        float width = 100;
        std::optional<float> autoResizeWidth;
        if (lua_width.get_type() == sol::type::number)
        {
            width = lua_width;
        }
        else if (lua_width.get_type() == sol::type::string)
        {
            std::string widthStr = lua_width;
            auto nodeSize = ParseNodeSize(widthStr);
            if (nodeSize.type == NodeSizeType::Pixel)
            {
                width = nodeSize.size;
            }
            else
            {
                // relative size
                autoResizeWidth = nodeSize.size / 100.0f;
            }
        }
        auto lua_height = luaNode["height"];
        float height = 100;
        std::optional<float> autoResizeHeight;
        if (lua_height.get_type() == sol::type::number)
        {
            height = lua_height;
        }
        else if (lua_height.get_type() == sol::type::string)
        {
            std::string heightStr = lua_height;
            auto nodeSize = ParseNodeSize(heightStr);
            if (nodeSize.type == NodeSizeType::Pixel)
            {
                height = nodeSize.size;
            }
            else
            {
                // relative size
                autoResizeHeight = nodeSize.size / 100.0f;
            }
        }
        float size = 36;
        auto lua_size = luaNode["size"];
        if (lua_size.get_type() == sol::type::number)
        {
            size = lua_size;
        }
        auto& base = hierarchy.GetComponent<BaseComponent>(node);
        base.size = Vec2f(width, height);
        auto lua_text = luaNode["text"];
        std::string text;
        if (lua_text.get_type() == sol::type::string)
        {
            text = lua_text;
        }
        else
        {
            return std::unexpected<std::string>("invalid text: not a string");
        }
        auto& textComponent = hierarchy.AddComponent<TextComponent>(node, text);
        textComponent.worldSize = size;
        if (autoResizeWidth || autoResizeHeight)
        {
            auto& autoResize = hierarchy.AddComponent<AutoResizeComponent>(node);
            if (autoResizeHeight)
            {
                autoResize.height = *autoResizeHeight;
                autoResize.EnableHeight();
            }
            if (autoResizeWidth)
            {
                autoResize.width = *autoResizeWidth;
                autoResize.EnableWidth();
            }
        }

        return node;
    }
};
} // namespace Aether::UI::Lua