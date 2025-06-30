#pragma once
#include "NodeCreator.h"
#include <sol/sol.hpp>
namespace Aether::UI
{
    class HierarchyLuaLoader
    {
    public:
        std::optional<std::string> LoadHierarchy(Hierarchy& hierarchy,const std::string& luaScript)
        {
            auto res=m_State.script(luaScript);
            if(!res.valid())
            {
                return std::format("Lua script result invalid");
            }
            if(!(res.get_type() == sol::type::table))
            {
                return std::format("Lua script result is not a table");
            }
            sol::table root = res;

            return std::nullopt;
        }
    private:
        std::expected<Node*, std::string> LoadNode(Hierarchy& hierarchy, sol::table node, const std::string& path)
        {
            std::string tag=node["tag"];
            std::string curPath = path + "/" + tag;
            auto iter = m_NodeCreators.find(tag);
            if (iter == m_NodeCreators.end())
            {
                return std::unexpected<std::string>(std::format("No node creator found for tag: {}", tag));
            }
            auto& creator = iter->second;
            auto newNodeEx = creator(hierarchy, node);
            if (!newNodeEx)
            {
                return std::unexpected<std::string>(std::format("Error creating node: {}, {}", curPath, newNodeEx.error()));
            }
            Node* newNode = newNodeEx.value();
            // Set node id if it exists
            if (node["id"].valid())
            {
                std::string id = node["id"];
                hierarchy.GetNodeIdMap()[id] = newNode;
            }
            // Process children
            if (node["children"].valid())
            {
                sol::table children = node["children"];
                for (const auto& child : children)
                {
                    auto childNodeEx = LoadNode(hierarchy, child.second, curPath);
                    if (!childNodeEx)
                    {
                        return std::unexpected<std::string>(std::format("Error loading child node: {}, {}", curPath, childNodeEx.error()));
                    }
                    newNode->PushChild(childNodeEx.value());
                }
            }
            return newNode;
        }
    private:
        std::unordered_map<std::string,HierarchyLuaLoaderNodeCreatorI> m_NodeCreators;
        sol::state m_State;
    };
}