#include "GlobalGuiImageCache.h"
#include "GlobalTextureCache.h"
namespace AetherEditor
{
void GlobalGuiImageCache::GetImageAsync(
    const std::string& path, bool srgb,
    const std::function<void(std::expected<LocalRef<ImGuiComponent::Image>, std::string>&&)>& onComplete)
{
    // is image in cache?
    auto cachedImage = m_ImageCache.Get(path);
    if (cachedImage)
    {
        // return cached image
        onComplete(std::expected<LocalRef<ImGuiComponent::Image>, std::string>{*cachedImage});
        return;
    }
    GlobalTextureCache::GetTextureAsync(
        path, srgb, [this, path, onComplete](std::expected<Ref<DeviceTexture>, std::string>&& res) -> void {
            if (!res)
            {
                onComplete(std::unexpected(std::format("failed to load texture: {}", res.error())));
            }
            auto image = ImGuiComponent::Image::Create(std::move(res.value()));
            if (!image)
            {
                onComplete(std::unexpected(std::format("failed to create ImGui image: {}", image.error())));
            }
            auto localRefImage = LocalRef<ImGuiComponent::Image>(new ImGuiComponent::Image(std::move(*image)));
            m_ImageCache.Put(path, localRefImage);
            m_ImageCache.TrimToCapacity(m_Capacity);
            onComplete(std::expected<LocalRef<ImGuiComponent::Image>, std::string>{localRefImage});
        });
}
} // namespace AetherEditor