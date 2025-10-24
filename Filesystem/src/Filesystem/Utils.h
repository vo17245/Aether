#pragma once
#include "File.h"
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
namespace Aether::Filesystem
{
std::optional<std::vector<uint8_t>> ReadFile(const Filesystem::Path& path);
std::optional<std::string> ReadFileToString(const Filesystem::Path& path);
/**
 * @brief 读取整个文件到buffer中,如果bufferSize小于文件大小,则返回false
 */
bool ReadFile(uint8_t* buffer, size_t bufferSize, const std::string_view path);
bool WriteFile(const Filesystem::Path& path, const std::span<const uint8_t> data);
bool WriteFile(const Filesystem::Path& path, const std::span<const char> data);
bool RemoveTree(const std::string_view path);
bool CreateDirectories(const Path& path);
std::vector<U8String> ListFiles(const std::string_view path);
/**
 * @return error msg if failed,otherwise return nullopt
*/
std::optional<std::string> CopyFile(const std::string& srcPath,const std::string& destPath,size_t bufferSize=8192);
} // namespace Aether::FIlesystem