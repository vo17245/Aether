#pragma once
#include <string>
#include <string_view>

#include <expected>
namespace Aether::UI
{
struct SelectFileParam
{
    std::string defaultPath;
};
std::expected<std::string, std::string> SyncSelectFileEx(const SelectFileParam& param);
std::expected<std::string, std::string> SyncSelectFile();
std::expected<std::string, std::string> SyncSelectDirectroy();
} // namespace Aether::UI