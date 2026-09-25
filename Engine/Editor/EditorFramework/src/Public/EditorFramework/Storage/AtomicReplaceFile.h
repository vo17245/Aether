#pragma once

#include <expected>
#include <filesystem>
#include <span>
#include <string>

namespace Aether::EditorFramework::Storage
{
using FileResult = std::expected<void, std::string>;

class AtomicFileOperations
{
public:
    virtual ~AtomicFileOperations() = default;
    virtual FileResult Write(const std::filesystem::path& temporary, std::span<const std::byte> contents) = 0;
    virtual FileResult Flush(const std::filesystem::path& temporary) = 0;
    virtual FileResult Replace(const std::filesystem::path& temporary, const std::filesystem::path& destination) = 0;
    virtual void Remove(const std::filesystem::path& path) noexcept = 0;
};

FileResult AtomicReplaceFile(const std::filesystem::path& destination,
                             std::span<const std::byte> contents,
                             AtomicFileOperations* operations = nullptr);
}
