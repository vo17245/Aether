#pragma once
#include <optional>
#include <vector>
#include <filesystem>
namespace Aether
{
	class FileUtils
	{
	public:
		static std::optional<std::vector<std::byte>> ReadFile(const std::string& path);
		static std::optional<std::vector<std::byte>> ReadFile(const std::filesystem::path& path);
	};
	
}
