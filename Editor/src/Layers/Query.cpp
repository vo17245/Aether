#include "Query.h"
#include <variant>
namespace 
{
    struct Select
    {
        std::string tag;
        size_t index=0;
    };
    std::vector<std::string> SplitPath(std::string_view path)
    {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = 0;
        while (end < path.size())
        {
            if (path[end] == '/')
            {
                if (start < end)
                {
                    result.emplace_back(path.substr(start, end - start));
                }
                start = end + 1;
            }
            end++;
        }
        if (start < end)
        {
            result.emplace_back(path.substr(start, end - start));
        }
        return result;
    }
    //example: /quad/quad[0]/text[1] 
    //         quad means quad[0]
    std::vector<Select> ParsePath(std::string_view path)
    {
        std::vector<Select> result;
        auto parts = SplitPath(path);
        for (const auto& part : parts)
        {
            Select select;
            size_t bracketPos = part.find('[');
            if (bracketPos != std::string::npos)
            {
                select.tag = part.substr(0, bracketPos);
                std::string indexStr = part.substr(bracketPos + 1);
                indexStr.pop_back(); // remove ']'
                select.index = std::stoul(indexStr);
            }
            else
            {
                select.tag = part;
            }
            result.push_back(select);
        }
        return result;
    }
    struct Node
    {
        tinyxml2::XMLNode* node;
        size_t index;
    };
    std::vector<Node> FindNodes(tinyxml2::XMLNode& root, std::string& tag)
    {
        std::vector<Node> nodes;
        size_t index=0;
        auto* child=root.FirstChild();
        while(child)
        {
            auto* element = child->ToElement();
            if(!element)
            {
                child= child->NextSibling();
                continue;
            }
            if(element->Name()==tag)
            {
                nodes.emplace_back(Node{child,index});
            }
            index++;
            child= child->NextSibling();
        }
        return nodes;
    }
}


UI::Node* QueryNode(UI::Hierarchy& hierarchy,tinyxml2::XMLDocument& doc,std::string_view path)
{
    std::vector<Node> nodePath;
    // search node path
    {
        auto selects= ParsePath(path);
        if (selects.empty())
        {
            return hierarchy.GetRoot();
        }
        tinyxml2::XMLNode* currentNode = doc.RootElement();
        if(!currentNode)
        {
            return nullptr; // root element not found
        }
        for(size_t i=1;i<selects.size();i++)
        {
            auto& select = selects[i];
            auto nodes=FindNodes(*currentNode,select.tag);
            if(nodes.size()<=select.index)
            {
                return nullptr; // index out of range
            }   
            auto& node = nodes[select.index];
            nodePath.emplace_back(node);
            currentNode = node.node;
        }
    }
    // find node in hierarchy
    UI::Node* node= hierarchy.GetRoot();
    assert(node && "Hierarchy root is null");
    {
        for(auto& n:nodePath)
        {
            if(node->children.size()<=n.index)
            {
                return nullptr; // index out of range
            }
            node=node->children[n.index];
        }
    }
    return node;
}