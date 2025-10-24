#pragma once
#include <Core/Core.h>
namespace Aether::Project
{
class Project;
class Asset
{
public:
    virtual ~Asset()=default;
    Asset()
    {
        m_ID = UUID::Create();
    }
    UUID GetID() const
    {
        return m_ID;
    }
    std::string GetTag() const
    {
        return m_Tag;
    }
    const std::string& GetAddress() const
    {
        return m_Address;
    }
    void SetAddress(const std::string& address)
    {
        m_Address = address;
    }


     
private:
    UUID m_ID;
    std::string m_Tag;
    /** Asset File Path Relative to Project Content Directory*/
    std::string m_Address;
};
} // namespace Aether::Project