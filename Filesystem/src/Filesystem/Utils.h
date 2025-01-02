#pragma once
#include "File.h"
#include <cstdint>
#include <optional>
#include <span>
namespace Aether {
namespace Filesystem {
std::optional<std::vector<char>> ReadFile(const Filesystem::Path& path);
bool WriteFile(const Filesystem::Path& path, const std::span<uint8_t> data);
bool WriteFile(const Filesystem::Path& path, const std::span<char> data);
}
} // namespace Aether::Filesystem