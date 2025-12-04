#pragma once
#include <Core/Core.h>
#include "AssetType.h"
namespace Aether::Project
{
class Project;

class Asset
{
public:
    virtual ~Asset()=default;
    Asset(AssetType type) : m_Type(type)
    {
        m_ID = UUID::Create();
    }
    UUID GetID() const
    {
        return m_ID;
    }
    const std::string& GetName() const
    {
        return m_Name;
    }
    void SetName(const std::string& name)
    {
        m_Name = name;
    }
    const std::string& GetAddress() const
    {
        return m_Address;
    }
    void SetAddress(const std::string& address)
    {
        m_Address = address;
    }

    AssetType GetType() const
    {
        return m_Type;
    }
     
private:
    AssetType m_Type;
    UUID m_ID;
    std::string m_Name;
    /** Asset File Path Relative to Project Content Directory*/
    std::string m_Address;
};
} // namespace Aether::Project