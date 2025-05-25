#pragma once
#include <optional>
#include <string>
#include <tinyxml2.h>
#include <functional>
#include <unordered_map>
#include "Hierarchy.h"
#include <vector>
#include <format>
#include <IO/WriterReader.h>
#include <expected>
#include "NodeCreator.h"
namespace Aether::UI
{

class HierarchyLoader
{
public:
    
    std::optional<std::string> LoadHierarchy(Hierarchy& hierarchy, const std::string& xmlContent)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError error = doc.Parse(xmlContent.c_str());
        if (error != tinyxml2::XML_SUCCESS)
        {
            return "Failed to parse XML: " + std::string(tinyxml2::XMLDocument::ErrorIDToName(error));
        }
        tinyxml2::XMLNode* root = doc.FirstChild();
        if (!root)
        {
            return "No root element found in XML.";
        }
        auto rootNodeEx = LoadNode(hierarchy, *root, "");
        if (!rootNodeEx)
        {
            return rootNodeEx.error();
        }
        hierarchy.SetRoot(rootNodeEx.value());
        return std::nullopt;
    }

    void PushNodeCreatorI(const std::string& tag, HierarchyLoaderNodeCreatorI&& creator)
    {
        m_NodeCreators[tag] = std::move(creator);
    }
    template<HierarchyLoaderNodeCreator Creator>
    void PushNodeCreator(const std::string& tag)
    {
        m_NodeCreators[tag]= [](Hierarchy& hierarchy, const tinyxml2::XMLNode& node) {
            return Creator()(hierarchy, node);
        };
    }

private:
    std::unordered_map<std::string, HierarchyLoaderNodeCreatorI> m_NodeCreators;
    std::expected<Node*, std::string> LoadNode(Hierarchy& hierarchy, const tinyxml2::XMLNode& node, const std::string& path)
    {
        std::string tag = node.Value();
        std::string curPath = path + "/" + tag;
        auto iter = m_NodeCreators.find(tag);
        if (iter == m_NodeCreators.end())
        {
            return std::unexpected<std::string>(std::format("Error: {}, Unknown node type\n", curPath));
        }
        auto newNodeEx = iter->second(hierarchy, node);
        if (!newNodeEx)
        {
            return std::unexpected<std::string>(std::format("Error: {}, {}\n", curPath, newNodeEx.error()));
        }
        const tinyxml2::XMLNode* child = node.FirstChild();
        while (child)
        {
            if(child->ToText())
            {
                child = child->NextSibling();
                continue;
            }
            auto childNodeEx = LoadNode(hierarchy, *child, curPath);
            if (!childNodeEx)
            {
                hierarchy.DestroyNode(newNodeEx.value());
                return std::unexpected<std::string>(childNodeEx.error());
            }
            newNodeEx.value()->PushChild(childNodeEx.value());

            child = child->NextSibling();
        }
        return newNodeEx.value();
    }
};
} // namespace Aether::UI