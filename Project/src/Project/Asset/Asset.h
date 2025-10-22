#pragma once
#include <Core/Core.h>
namespace Aether::Project
{
class Asset
{
public:
    Asset()
    {
        m_ID = UUID::Create();
    }
    UUID GetID() const
    {
        return m_ID;
    }

private:
    UUID m_ID;
};
} // namespace Aether::Project