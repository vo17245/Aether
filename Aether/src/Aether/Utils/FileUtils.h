#pragma once
#include <optional>
#include <string>
#include <vector>
#include <filesystem>
namespace Aether
{
	class FileUtils
	{
	public:
		static std::optional<std::vector<std::byte>> ReadFile(const std::string& path);
		static std::optional<std::vector<std::byte>> ReadFile(const std::filesystem::path& path);
		static std::optional<std::string> ReadFileAsString(const std::filesystem::path& path);
		static std::optional<std::string> ReadFileAsString(const std::string& path);

	};
	
}
