#pragma once
#include <unordered_map>
#include <filesystem>
#include <optional>
#include "../Resource/ModelAsset.h"
#include "../Resource/ModelAssetImporter.h"
#include <mutex>

namespace Aether
{
	
	template <typename T>
	class FileCache
	{
		static_assert(sizeof(T) == 0);
	};
	template <>
	class FileCache<ModelAsset>
	{
	public:
		static std::optional<Ref<ModelAsset>> Get(const std::filesystem::path& path);
	private:
		static std::unordered_map<std::filesystem::path, Ref<ModelAsset>> s_Cache;
		static std::mutex s_Mutex;
	};
	inline std::optional<Ref<ModelAsset>> FileCache<ModelAsset>::Get(const std::filesystem::path& path)
	{
		std::lock_guard guard(s_Mutex);
		auto iter = s_Cache.find(path);
		if (iter != s_Cache.end())
		{
			return iter->second;
		}
		ModelAssetImporter importer;
		auto res=importer.LoadFromFile(path.string());
		if (!res)
		{
			return std::nullopt;
		}
		s_Cache[path] = res.value();
		return res.value();
	}
}