#pragma once
#include "Hierarchy.h"
#include "HierarchyLoader.h"
#include "Component/Base.h"
#include "Component/Node.h"
#include "Component/Quad.h"
#include "Node.h"
#include "tinyxml2.h"
#include <cstdlib>
#include <IO/WriterReader.h>
namespace Aether::UI
{
struct QuadNodeCreator
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
    std::expected<Node*, std::string> operator()(Hierarchy& hierarchy, const tinyxml2::XMLNode& node)
    {
        auto* newNode = hierarchy.CreateNode();
        UI::QuadDesc desc;

        const char* color = node.ToElement()->Attribute("color");
        if (color)
        {
            Vec4f c;
            auto err = ParseColor(color, c);
            if (err)
            {
                hierarchy.DestroyNode(newNode);
                return std::unexpected<std::string>(err.value());
            }
            desc.color = c;
        }
        hierarchy.GetScene().AddComponent<QuadComponent>(newNode->id, desc);
        auto& base = hierarchy.GetScene().GetComponent<BaseComponent>(newNode->id);
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
} // namespace Aether::UI