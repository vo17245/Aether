#pragma once
#include <vector>
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

private:
    std::vector<ContentTreeNode*> m_Contents;
};
}
