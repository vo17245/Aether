#pragma once
#include "NodeCreator.h"
#include <sol/sol.hpp>
namespace Aether::UI::Lua
{
class HierarchyLoader
{
public:
    void PushNodeCreatorI(const std::string& tag, HierarchyLuaLoaderNodeCreatorI&& creator)
    {
        m_NodeCreators[tag] = std::move(creator);
    }
    template <HierarchyLuaLoaderNodeCreator Creator>
    void PushNodeCreator(const std::string& tag)
    {
        m_NodeCreators[tag] = [](Hierarchy& hierarchy, sol::table& node) {
            return Creator()(hierarchy, node);
        };
    }
    std::optional<std::string> LoadHierarchy(Hierarchy& hierarchy, const std::string& luaScript)
    {
        sol::protected_function_result res;
        try
        {
            res = m_State.script(luaScript);
        }
        catch (const sol::error& e)
        {
            return std::format("Lua script error: {}", e.what());
        }
        if (!res.valid())
        {
            return std::format("Lua script result invalid");
        }
        if (!(res.get_type() == sol::type::table))
        {
            return std::format("Lua script result is not a table");
        }
        sol::table root = res;
        auto rootNodeEx = LoadNode(hierarchy, root, "/");

        if (!rootNodeEx)
        {
            return rootNodeEx.error();
        }
        hierarchy.SetRoot(rootNodeEx.value());

        return std::nullopt;
    }

private:
    std::expected<Node*, std::string> LoadNode(Hierarchy& hierarchy, sol::table node, const std::string& path)
    {
        std::string tag = node["type"];
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
            if(hierarchy.GetNodeIdMap().find(id)!=hierarchy.GetNodeIdMap().end())
            {
                return std::unexpected<std::string>(std::format("Node id {} already exists", id));
            }
            hierarchy.GetNodeIdMap()[id] = newNode;
        }
        // Process children
        if (node["content"].valid())
        {
            sol::table content = node["content"];
            for (const auto& child : content)
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
    std::unordered_map<std::string, HierarchyLuaLoaderNodeCreatorI> m_NodeCreators;
    sol::state m_State;
};
} // namespace Aether::UI::Lua