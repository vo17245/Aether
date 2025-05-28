#pragma once
#include <UI/Hierarchy/Hierarchy.h>
#include <UI/Hierarchy/Component/Base.h>
#include <UI/Hierarchy/Component/Text.h>
#include <UI/Hierarchy/Component/Mouse.h>
#include <UI/Hierarchy/Component/Quad.h>
#include <UI/Hierarchy/Node.h>
#include <tinyxml2.h>
namespace Aether
{

struct ButtonNodeCreator
{
    std::optional<std::string> ParseColor(const char* color, Vec4f& res)
    {
        size_t begin = 0;
        size_t end = 0;
        const char* p = color;
        size_t fc = 0;
        while (p[end])
        {
            if (p[end] == ',')
            {
                std::string s(color + begin, end - begin);
                float v;
                try
                {
                    v = std::atof(s.c_str());
                }
                catch (std::exception& e)
                {
                    return std::format("Error: {}, {}", color, e.what());
                }
                if (fc == 0)
                    res.x() = v;
                else if (fc == 1)
                    res.y() = v;
                else if (fc == 2)
                    res.z() = v;
                else if (fc == 3)
                    res.w() = v;
                fc++;
                end++;
                begin = end;
            }
            else
            {
                end++;
            }
        }
        if (begin != end)
        {
            std::string s(color + begin, end - begin);
            float v;
            try
            {
                v = std::atof(s.c_str());
            }
            catch (std::exception& e)
            {
                return std::format("Error: {}, {}", color, e.what());
            }
            if (fc == 0)
                res.x() = v;
            else if (fc == 1)
                res.y() = v;
            else if (fc == 2)
                res.z() = v;
            else if (fc == 3)
                res.w() = v;
            fc++;
        }
        if (fc != 4)
        {
            return "Invalid color format";
        }
        return std::nullopt;
    }
    std::expected<UI::Node*, std::string> operator()(UI::Hierarchy& hierarchy, const tinyxml2::XMLNode& node)
    {
        auto* newNode = hierarchy.CreateNode();
        auto& scene = hierarchy.GetScene();
        // quad component
        Vec4f color(1, 1, 1, 1); // default white color
        const char* colorAttr = node.ToElement()->Attribute("color");
        if (colorAttr)
        {
            auto colorOpt = ParseColor(colorAttr, color);
            if (colorOpt)
            {
                hierarchy.DestroyNode(newNode);
                return std::unexpected<std::string>(colorOpt.value());
            }
        }
        UI::QuadDesc desc;
        desc.color = color;
        scene.AddComponent<UI::QuadComponent>(newNode->id, desc);
        // mouse component
        hierarchy.AddComponent<UI::MouseComponent>(newNode);
        // text component world size and content
        UI::TextComponent tc;
        tc.content = node.ToElement()->GetText();
        const char* worldSize = node.ToElement()->Attribute("fontSize");
        float worldSizeValue = 16.0f;// default font size
        if (worldSize)
        {
            worldSizeValue= std::atof(worldSize);
            
        }
        tc.worldSize = worldSizeValue;
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
