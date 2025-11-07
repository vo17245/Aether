#include "GlobalTextureCache.h"
#include <AetherEditor/Utils/LoadTexture.h>
namespace AetherEditor
{
GlobalTextureCache& GlobalTextureCache::GetSingleton()
{
    static GlobalTextureCache instance;
    return instance;
}
void GlobalTextureCache::SetCapacityImpl(size_t capacity)
{
    m_Capacity = capacity;
    m_TextureCache.TrimToCapacity(m_Capacity);
}
void GlobalTextureCache::GetTextureAsyncImpl(const std::string& path, bool srgb,
                     const std::function<void(std::expected<Ref<DeviceTexture>, std::string>&&)>& onComplete)
{
    // is texture in cache?
    auto cachedTexture = m_TextureCache.Get(path);
    if (cachedTexture)
    {
        // return cached texture
        onComplete(std::expected<Ref<DeviceTexture>, std::string>{*cachedTexture});
        return;
    }   
    // load texture asynchronously
    GlobalThreadPool::Enqueue(
        [path, srgb]() -> std::expected<Ref<DeviceTexture>, std::string> {
            PixelFormat format = srgb ? PixelFormat::RGBA8888_SRGB : PixelFormat::RGBA8888;
            auto loadRes = Utils::LoadTexture(path, format);
            if (!loadRes)
            {
                return std::unexpected(loadRes.error());
            }
            return CreateRef<DeviceTexture>(std::move(*loadRes));
        },
        [this,onComplete,path](std::expected<Ref<DeviceTexture>, std::string>&& res) 
        { 
            // if load success, add to cache
            if(res)
            {
                // add to cache
                m_TextureCache.Put(path, res.value());
                m_TextureCache.TrimToCapacity(m_Capacity);
            }
            onComplete(std::move(res)); 
        });
}
} // namespace AetherEditor