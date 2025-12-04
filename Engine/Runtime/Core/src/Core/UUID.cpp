#include "UUID.h"
namespace Aether
{
namespace
{
class UUIDGenerator
{
public:
    UUIDGenerator()
    {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        m_Generator = uuids::uuid_random_generator{generator};
    }
    uuids::uuid Generate()
    {
        return (*m_Generator)();
    }
    static UUIDGenerator& Get()
    {
        thread_local static UUIDGenerator instance;
        return instance;
    }

private:
    std::optional<uuids::uuid_random_generator> m_Generator;
};
} // namespace
UUID UUID::Create()
{
    UUID uuid;
    uuid.m_UUID = UUIDGenerator::Get().Generate();
    return uuid;
}
std::string UUID::ToString() const
{
    return uuids::to_string(m_UUID);
};
std::optional<UUID> UUID::FromString(const std::string& uuidString)
{
    auto opt = uuids::uuid::from_string(uuidString);
    if (!opt.has_value())
    {
        return std::nullopt;
    }
    UUID uuid;
    uuid.m_UUID = opt.value();
    return uuid;
}
} // namespace Aether