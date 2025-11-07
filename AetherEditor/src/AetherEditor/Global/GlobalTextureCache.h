#pragma once
#include <Core/Core.h>
#include <Render/Render.h>
#include <Async/GlobalThreadPool.h>
using namespace Aether;
namespace AetherEditor
{
class GlobalTextureCache
{
public:
    static void SetCapacity(size_t capacity)
    {
        GetSingleton().SetCapacityImpl(capacity);
    }

    static void GetTextureAsync(const std::string& path, bool srgb,
                                const std::function<void(std::expected<Ref<DeviceTexture>, std::string>&&)>& onComplete)
    {
        GetSingleton().GetTextureAsyncImpl(path, srgb, onComplete);
    }
    static inline void Clear()
    {
        GetSingleton().ClearImpl();
    }

private:
    static GlobalTextureCache& GetSingleton();
    void SetCapacityImpl(size_t capacity);

    void GetTextureAsyncImpl(const std::string& path, bool srgb,
                             const std::function<void(std::expected<Ref<DeviceTexture>, std::string>&&)>& onComplete);
    inline void ClearImpl()
    {
        m_TextureCache.Clear();
    }
    LRUCache<std::string, Ref<DeviceTexture>> m_TextureCache;
    size_t m_Capacity = 10;
};
} // namespace AetherEditor