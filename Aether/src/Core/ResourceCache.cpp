#include "ResourceCache.h"

namespace Aether
{
	std::unordered_map<std::filesystem::path, Ref<ModelAsset>> FileCache<ModelAsset>::s_Cache;
	std::mutex FileCache<ModelAsset>::s_Mutex;
}
