#pragma once
#include <AetherEditor/UIComponent/Image.h>
#include "GlobalTextureCache.h"
namespace AetherEditor
{
class GlobalGuiImageCache
{
public:
    void SetCapacity(size_t capacity)
    {
        m_ImageCache.TrimToCapacity(capacity);
    }
    void
    GetImageAsync(const std::string& path,bool srgb,
                  const std::function<void(std::expected<LocalRef<ImGuiComponent::Image>, std::string>&&)>& onComplete);
    inline void Clear()
    {
        m_ImageCache.Clear();
    }

private:
    LRUCache<std::string, LocalRef<ImGuiComponent::Image>> m_ImageCache;
    size_t m_Capacity = 50;
};
} // namespace AetherEditor