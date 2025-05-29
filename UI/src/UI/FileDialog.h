#pragma once
#include <string>
#include <string_view>

#include <expected>
namespace Aether::UI
{
    std::expected<std::string,std::string> SyncSelectFile();
}