#include "FileUtils.h"
#include "../Core/Log.h"
#include <fstream>
namespace Aether
{
    std::optional<std::vector<std::byte>> FileUtils::ReadFile(const std::string& path)
    {
        return ReadFile(std::filesystem::path(path));
    }

    std::optional<std::vector<std::byte>> FileUtils::ReadFile(const std::filesystem::path& path)
    {
        std::ifstream ifs(path, std::ios::binary | std::ios::in);
        if (!ifs.is_open()) {
            AETHER_DEBUG_LOG_ERROR("failed to load {}", path.string());
            return std::nullopt;
        }
        ifs.seekg(0, std::ios::end);
        std::size_t file_size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::vector<std::byte> data(file_size);
        if (!ifs.read(reinterpret_cast<char*>(data.data()), file_size)) {
            AETHER_DEBUG_LOG_ERROR("failed to read file {}", path.string());
            return std::nullopt;
        }
        return data;
    }

}
