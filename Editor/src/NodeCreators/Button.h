#pragma once
#include <UI/Hierarchy/Hierarchy.h>
#include <UI/Hierarchy/Component/Base.h>
#include <UI/Hierarchy/Component/Text.h>
#include <UI/Hierarchy/Component/Mouse.h>
#include <UI/Hierarchy/Node.h>
#include <tinyxml2.h>
namespace Aether
{

struct ButtonNodeCreator
{
    std::expected<UI::Node*, std::string> operator()(UI::Hierarchy& hierarchy, const tinyxml2::XMLNode& node)
    {
        auto* newNode = hierarchy.CreateNode();
        auto& scene = hierarchy.GetScene();
        // mouse component
        hierarchy.AddComponent<UI::MouseComponent>(newNode);
        // text component world size and content
        UI::TextComponent tc;
        tc.content = node.ToElement()->GetText();
        const char* worldSize = node.ToElement()->Attribute("fontSize");
        if (worldSize)
        {
            tc.worldSize = std::atof(worldSize);
        }
        hierarchy.GetScene().AddComponent<UI::TextComponent>(newNode->id, std::move(tc));
        // base component width height
        auto& base = hierarchy.GetScene().GetComponent<UI::BaseComponent>(newNode->id);
        const char* width = node.ToElement()->Attribute("width");

        if (width)
        {
            float w = 0;
            try
            {
                w = std::atof(width);
            }
            catch (...)
            {
                hierarchy.DestroyNode(newNode);
                return std::unexpected<std::string>(std::format("Error: {}, {}", width, "Invalid width format"));
            }
            base.size.x() = w;
        }
        const char* height = node.ToElement()->Attribute("height");
        if (height)
        {
            float h = 0;
            try
            {
                h = std::atof(height);
            }
            catch (...)
            {
                hierarchy.DestroyNode(newNode);
                return std::unexpected<std::string>(std::format("Error: {}, {}", height, "Invalid height format"));
            }
            base.size.y() = h;
        }
        return newNode;
    }
};
} // namespace Aether
