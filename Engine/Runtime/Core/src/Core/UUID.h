#pragma once
#include <uuid.h>

namespace Aether
{
class UUID
{
public:
    /**
     * @brief Create a new UUID
    */
    static UUID Create();
    /**
     * @brief Create a UUID from string representation
    */
    static std::optional<UUID> FromString(const std::string& uuidString);
    /**
     * @brief Convert the UUID to string representation
    */
    std::string ToString()const;
private:
    uuids::uuid m_UUID;
};
}