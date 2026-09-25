#pragma once

#include <Core/DisplaySurfaceToken.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <Render/RenderGraph/Resource/ResourceArena.h>
#include <Render/RHI/Sampler.h>
#include <Render/RHI/Texture2D.h>

#include <atomic>
#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Aether::Render
{
struct DisplaySurfaceExtent
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct DisplaySurfaceUiPublication
{
    DisplaySurfaceToken token;
    DisplaySurfaceExtent extent;
    // CPU-only pin. It owns no Vulkan object and may be copied by the UI thread.
    std::shared_ptr<const void> lifetimePin;
};

struct DisplaySurfaceGpuBinding
{
    rhi::Texture2D* texture = nullptr;
    rhi::TextureView* view = nullptr;
    rhi::Sampler* sampler = nullptr;
    RenderGraph::ResourceId<rhi::Texture2D> resourceId{};
    std::shared_ptr<const void> lease;
};

// Window-owned GPU resources. Create/Replace/Retire/Import/Resolve/Collect and
// Shutdown run on the render owner. AcquireUiPublication is thread-safe and
// exposes only immutable metadata and a CPU lifetime pin.
class DisplaySurfaceService final
{
public:
    DisplaySurfaceService(RenderGraph::ResourceArena& arena, std::uint32_t frameSlotCount);
    ~DisplaySurfaceService();
    DisplaySurfaceService(const DisplaySurfaceService&) = delete;
    DisplaySurfaceService& operator=(const DisplaySurfaceService&) = delete;

    std::expected<void, std::string> Initialize();
    std::expected<DisplaySurfaceToken, std::string> CreateSurface(DisplaySurfaceExtent extent);
    std::expected<DisplaySurfaceToken, std::string> ReplaceSurface(
        DisplaySurfaceToken current, DisplaySurfaceExtent extent);
    bool RetireSurface(DisplaySurfaceToken token) noexcept;
    std::optional<DisplaySurfaceUiPublication> AcquireUiPublication(DisplaySurfaceToken token) const;

    std::optional<RenderGraph::AccessId<rhi::Texture2D>> ImportSurface(
        RenderGraph::RenderGraph& graph, DisplaySurfaceToken token);
    DisplaySurfaceGpuBinding ResolveForUi(DisplaySurfaceToken token,
        const std::shared_ptr<const void>& lifetimePin, std::uint32_t frameSlot) noexcept;
    void CollectRetired() noexcept;
    void ShutdownAfterGpuIdle() noexcept;
    std::uint64_t FallbackResolveCount() const noexcept
    { return m_FallbackResolveCount.load(std::memory_order_relaxed); }
    std::uint32_t FrameSlotCount() const noexcept { return m_FrameSlotCount; }

private:
    struct TokenHash
    {
        std::size_t operator()(DisplaySurfaceToken token) const noexcept;
    };
    struct SurfaceResources;
    using ResourcesPtr = std::shared_ptr<SurfaceResources>;

    std::expected<ResourcesPtr, std::string> CreateResources(
        DisplaySurfaceToken token, DisplaySurfaceExtent extent, bool publishPin);
    std::expected<void, std::string> CreateFallback();
    DisplaySurfaceGpuBinding FallbackBinding(std::uint32_t frameSlot) noexcept;
    void DestroyResources(const ResourcesPtr& resources) noexcept;

    RenderGraph::ResourceArena& m_Arena;
    const std::uint32_t m_FrameSlotCount;
    std::uint64_t m_NextId = 1;
    bool m_Initialized = false;
    std::unordered_map<DisplaySurfaceToken, ResourcesPtr, TokenHash> m_Resources;
    std::unordered_map<std::uint64_t, DisplaySurfaceToken> m_Current;
    ResourcesPtr m_Fallback;
    mutable std::mutex m_PublicationMutex;
    std::unordered_map<DisplaySurfaceToken, DisplaySurfaceUiPublication, TokenHash> m_Publications;
    std::atomic<std::uint64_t> m_FallbackResolveCount{0};
};
} // namespace Aether::Render
