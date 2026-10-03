#pragma once

#include <filesystem>
#include <string_view>

namespace Aether::Script::Detail
{
// Internal hostfxr adapter. All temporary storage failures print and abort.
class TemporaryRuntimeConfig
{
public:
    explicit TemporaryRuntimeConfig(std::string_view contents);
    ~TemporaryRuntimeConfig() noexcept;
    TemporaryRuntimeConfig(const TemporaryRuntimeConfig&) = delete;
    TemporaryRuntimeConfig& operator=(const TemporaryRuntimeConfig&) = delete;

    const std::filesystem::path& Path() const noexcept { return m_Path; }

private:
    std::filesystem::path m_Directory;
    std::filesystem::path m_Path;
};
}
