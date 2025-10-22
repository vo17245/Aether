#pragma once
#include <vector>
#include <string>
namespace Aether::Project
{

enum class ContentTreeNodeType
{
    Folder,
    Mesh,
    Texture,
};
class ContentTreeNode
{
public:
    ContentTreeNode(ContentTreeNodeType type) : m_Type(type)
    {
    }
    virtual ~ContentTreeNode() = default;
private:
    ContentTreeNodeType m_Type;
};
class Folder : public ContentTreeNode
{
public:
    Folder() : ContentTreeNode(ContentTreeNodeType::Folder)
    {
    }
    std::string& GetName()
    {
        return m_Name;
    }
    void SetName(const std::string& name)
    {
        m_Name = name;
    }
private:
    std::vector<ContentTreeNode*> m_Contents;
    std::string m_Name;
};
}
