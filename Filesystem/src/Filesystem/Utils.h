#pragma once
#include "File.h"
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
namespace Aether {
namespace Filesystem {
std::optional<std::vector<char>> ReadFile(const Filesystem::Path& path);
bool WriteFile(const Filesystem::Path& path, const std::span<uint8_t> data);
bool WriteFile(const Filesystem::Path& path, const std::span<char> data);
bool RemoveTree(const std::string_view path);
bool CreateDirectories(const Path& path);
std::vector<U8String> ListFiles(const std::string_view path);
}
} // namespace Aether::Filesystem