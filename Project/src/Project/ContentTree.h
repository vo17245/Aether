#pragma once
#include <vector>
#include <string>
#include "Asset/Asset.h"
namespace Aether::Project
{

enum class ContentTreeNodeType
{
    Folder,
    Asset,
};
class ContentTreeNode
{
public:
    ContentTreeNode(ContentTreeNodeType type) : m_Type(type)
    {
    }
    virtual ~ContentTreeNode() = default;
    ContentTreeNodeType GetType() const
    {
        return m_Type;
    }
    virtual std::string_view GetName() const = 0;
    ContentTreeNode* FindChildByName(const std::string& name)
    {
        for (auto& child : m_Children)
        {
            if (child->GetName() == name)
            {
                return child.get();
            }
        }
        return nullptr;
    }
    void AddChild(Scope<ContentTreeNode>&& child)
    {
        m_Children.emplace_back(std::move(child));
    }
    const std::vector<Scope<ContentTreeNode>>& GetChildren() const
    {
        return m_Children;
    }
    std::vector<Scope<ContentTreeNode>>& GetChildren()
    {
        return m_Children;
    }
    ContentTreeNode* GetParent() const
    {
        return m_Parent;
    }
    void SetParent(ContentTreeNode* parent)
    {
        m_Parent = parent;
    }

private:
    ContentTreeNodeType m_Type;
    ContentTreeNode* m_Parent = nullptr;
    std::vector<Scope<ContentTreeNode>> m_Children;
};
class Folder : public ContentTreeNode
{
public:
    Folder() : ContentTreeNode(ContentTreeNodeType::Folder)
    {
    }
    Folder(const std::string& name) : ContentTreeNode(ContentTreeNodeType::Folder), m_Name(name)
    {
    }
    std::string_view GetName() const override
    {
        return m_Name;
    }
    void SetName(const std::string& name)
    {
        m_Name = name;
    }

private:
    std::string m_Name;
};
class AssetContentNode : public ContentTreeNode
{
public:
    AssetContentNode() : ContentTreeNode(ContentTreeNodeType::Asset)
    {
    }
    Asset* GetAsset()
    {
        return m_Asset;
    }
    void SetAsset(Asset* asset)
    {
        m_Asset = asset;
    }
    virtual std::string_view GetName() const override
    {
        if (m_Asset)
        {
            return m_Asset->GetName();
        }
        return "<null asset>";
    }
    const std::string& GetAssetAddress() const
    {
        return m_AssetAddress;
    }
    void SetAssetAddress(const std::string& address)
    {
        m_AssetAddress = address;
    }

private:
    Asset* m_Asset = nullptr; // non-owning
    std::string m_AssetAddress;
};
} // namespace Aether::Project
