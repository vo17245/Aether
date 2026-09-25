#include <Render/DisplaySurfaceService.h>

#include <algorithm>
#include <array>
#include <limits>
#include <span>
#include <stdexcept>

namespace Aether::Render
{
namespace
{
constexpr std::uint64_t MaxSurfacePixels = 4096ull * 4096ull;

struct SurfacePin
{
    explicit SurfacePin(DisplaySurfaceToken value) : token(value) {}
    DisplaySurfaceToken token;
};

std::string TokenTag(DisplaySurfaceToken token)
{
    return "DisplaySurface." + std::to_string(token.id) + "." + std::to_string(token.generation);
}
}

struct DisplaySurfaceService::SurfaceResources
{
    DisplaySurfaceToken token;
    DisplaySurfaceExtent extent;
    std::shared_ptr<const void> pin;
    std::vector<rhi::Texture2D> textures;
    std::vector<rhi::TextureView> views;
    std::vector<RenderGraph::ResourceId<rhi::Texture2D>> textureIds;
    rhi::Sampler sampler;
    bool retired = false;
};

std::size_t DisplaySurfaceService::TokenHash::operator()(DisplaySurfaceToken token) const noexcept
{
    const auto first = std::hash<std::uint64_t>{}(token.id);
    const auto second = std::hash<std::uint32_t>{}(token.generation);
    return first ^ (second + 0x9e3779b9u + (first << 6) + (first >> 2));
}

DisplaySurfaceService::DisplaySurfaceService(RenderGraph::ResourceArena& arena, std::uint32_t frameSlotCount)
    : m_Arena(arena), m_FrameSlotCount(frameSlotCount)
{
}

DisplaySurfaceService::~DisplaySurfaceService() = default;

std::expected<DisplaySurfaceService::ResourcesPtr, std::string> DisplaySurfaceService::CreateResources(
    DisplaySurfaceToken token, DisplaySurfaceExtent extent, bool publishPin)
{
    if (!token.IsValid() || extent.width == 0 || extent.height == 0
        || std::uint64_t(extent.width) * extent.height > MaxSurfacePixels)
        return std::unexpected("display surface extent is invalid or exceeds the 4096x4096 limit");
    auto resources = std::make_shared<SurfaceResources>();
    resources->token = token;
    resources->extent = extent;
    if (publishPin) resources->pin = std::make_shared<SurfacePin>(token);
    resources->textures.reserve(m_FrameSlotCount);
    resources->views.reserve(m_FrameSlotCount);
    resources->textureIds.reserve(m_FrameSlotCount);
    resources->sampler = rhi::Sampler::CreateDefault();
    if (!resources->sampler) return std::unexpected("display surface sampler creation failed");

    try
    {
        for (std::uint32_t slot = 0; slot < m_FrameSlotCount; ++slot)
        {
            rhi::TextureDesc desc{
                .usages = PackFlags(rhi::TextureUsage::ColorAttachment, rhi::TextureUsage::Sample),
                .pixelFormat = PixelFormat::RGBA8888,
                .width = extent.width,
                .height = extent.height,
                .layout = rhi::TextureLayout::ShaderReadOnly,
            };
            auto texture = rhi::Texture2D::Create(desc);
            if (!texture) throw std::runtime_error("display surface color texture creation failed");
            auto view = texture.CreateImageView(rhi::TextureViewDesc{});
            if (!view) throw std::runtime_error("display surface image view creation failed");
            resources->textures.push_back(std::move(texture));
            resources->views.push_back(std::move(view));
            resources->textureIds.push_back(m_Arena.Import(&resources->textures.back()));
        }
    }
    catch (const std::exception& error)
    {
        for (const auto id : resources->textureIds) m_Arena.Destroy(id);
        resources->textureIds.clear();
        return std::unexpected(error.what());
    }
    return resources;
}

std::expected<void, std::string> DisplaySurfaceService::Initialize()
{
    if (m_Initialized) return std::unexpected("display surface service is already initialized");
    if (m_FrameSlotCount == 0) return std::unexpected("display surface service requires frame slots");
    m_Initialized = true;
    return {};
}

std::expected<void, std::string> DisplaySurfaceService::CreateFallback()
{
    if (m_Fallback) return {};
    auto fallback = std::make_shared<SurfaceResources>();
    try
    {
        fallback->token = {std::numeric_limits<std::uint64_t>::max(), 1};
        fallback->extent = {1, 1};
        fallback->textures.reserve(m_FrameSlotCount);
        fallback->views.reserve(m_FrameSlotCount);
        fallback->textureIds.reserve(m_FrameSlotCount);
        fallback->sampler = rhi::Sampler::CreateDefault();
        if (!fallback->sampler) return std::unexpected("display surface fallback sampler creation failed");
        const std::array<std::uint8_t, 4> pixel{255, 0, 255, 255};
        auto staging = rhi::StagingBuffer::Create(pixel.size());
        if (!staging) return std::unexpected("display surface fallback staging allocation failed");
        staging.SetData(0, pixel);
        for (std::uint32_t slot = 0; slot < m_FrameSlotCount; ++slot)
        {
            rhi::TextureDesc desc{
                .usages = PackFlags(rhi::TextureUsage::Sample, rhi::TextureUsage::TransferDst),
                .pixelFormat = PixelFormat::RGBA8888,
                .width = 1,
                .height = 1,
                .layout = rhi::TextureLayout::TransferDst,
            };
            auto texture = rhi::Texture2D::Create(desc);
            if (!texture) throw std::runtime_error("display surface fallback texture creation failed");
            texture.GetVk().SyncCopyBuffer(staging.GetVk());
            texture.SyncTransitionLayout(rhi::TextureLayout::TransferDst, rhi::TextureLayout::ShaderReadOnly);
            auto view = texture.CreateImageView(rhi::TextureViewDesc{});
            if (!view) throw std::runtime_error("display surface fallback view creation failed");
            fallback->textures.push_back(std::move(texture));
            fallback->views.push_back(std::move(view));
            fallback->textureIds.push_back(m_Arena.Import(&fallback->textures.back()));
        }
        m_Fallback = std::move(fallback);
        return {};
    }
    catch (const std::exception& error)
    {
        DestroyResources(fallback);
        return std::unexpected(std::string("display surface initialization failed: ") + error.what());
    }
}

std::expected<DisplaySurfaceToken, std::string> DisplaySurfaceService::CreateSurface(DisplaySurfaceExtent extent)
{
    if (!m_Initialized) return std::unexpected("display surface service is not initialized");
    if (!m_Fallback)
    {
        auto fallback = CreateFallback();
        if (!fallback) return std::unexpected(fallback.error());
    }
    if (m_NextId == 0 || m_NextId == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected("display surface token IDs are exhausted");
    const DisplaySurfaceToken token{m_NextId++, 1};
    auto resources = CreateResources(token, extent, true);
    if (!resources) return std::unexpected(resources.error());
    m_Resources.emplace(token, *resources);
    m_Current.emplace(token.id, token);
    {
        std::lock_guard lock(m_PublicationMutex);
        m_Publications.emplace(token, DisplaySurfaceUiPublication{token, extent, (*resources)->pin});
    }
    return token;
}

std::expected<DisplaySurfaceToken, std::string> DisplaySurfaceService::ReplaceSurface(
    DisplaySurfaceToken current, DisplaySurfaceExtent extent)
{
    const auto currentIt = m_Current.find(current.id);
    if (!current.IsValid() || currentIt == m_Current.end() || currentIt->second != current)
        return std::unexpected("display surface token is stale");
    if (current.generation == std::numeric_limits<std::uint32_t>::max())
        return std::unexpected("display surface generation is exhausted");
    const DisplaySurfaceToken replacement{current.id, current.generation + 1};
    auto resources = CreateResources(replacement, extent, true);
    if (!resources) return std::unexpected(resources.error());
    const auto inserted = m_Resources.emplace(replacement, *resources);
    if (!inserted.second)
    {
        DestroyResources(*resources);
        return std::unexpected("display surface generation already exists");
    }
    m_Resources.at(current)->retired = true;
    currentIt->second = replacement;
    {
        std::lock_guard lock(m_PublicationMutex);
        m_Publications.erase(current);
        m_Publications.emplace(replacement, DisplaySurfaceUiPublication{replacement, extent, (*resources)->pin});
    }
    return replacement;
}

bool DisplaySurfaceService::RetireSurface(DisplaySurfaceToken token) noexcept
{
    const auto found = m_Resources.find(token);
    const auto current = m_Current.find(token.id);
    if (found == m_Resources.end() || current == m_Current.end() || current->second != token) return false;
    found->second->retired = true;
    m_Current.erase(current);
    {
        std::lock_guard lock(m_PublicationMutex);
        m_Publications.erase(token);
    }
    return true;
}

std::optional<DisplaySurfaceUiPublication> DisplaySurfaceService::AcquireUiPublication(
    DisplaySurfaceToken token) const
{
    std::lock_guard lock(m_PublicationMutex);
    const auto found = m_Publications.find(token);
    return found == m_Publications.end() ? std::nullopt : std::optional(found->second);
}

std::optional<RenderGraph::AccessId<rhi::Texture2D>> DisplaySurfaceService::ImportSurface(
    RenderGraph::RenderGraph& graph, DisplaySurfaceToken token)
{
    if (&graph.GetResourceArena() != &m_Arena) return std::nullopt;
    const auto found = m_Resources.find(token);
    if (found == m_Resources.end() || found->second->retired) return std::nullopt;
    auto& resources = *found->second;
    RenderGraph::TextureDesc desc{
        .usages = PackFlags(rhi::TextureUsage::ColorAttachment, rhi::TextureUsage::Sample),
        .pixelFormat = PixelFormat::RGBA8888,
        .width = resources.extent.width,
        .height = resources.extent.height,
        .layout = rhi::TextureLayout::ShaderReadOnly,
    };
    graph.RetainLifetime(found->second);
    return graph.Import(TokenTag(token), desc,
        std::span<const RenderGraph::ResourceId<rhi::Texture2D>>(resources.textureIds));
}

DisplaySurfaceGpuBinding DisplaySurfaceService::FallbackBinding(std::uint32_t frameSlot) noexcept
{
    if (!m_Fallback || frameSlot >= m_Fallback->textures.size()) return {};
    return {&m_Fallback->textures[frameSlot], &m_Fallback->views[frameSlot], &m_Fallback->sampler,
            m_Fallback->textureIds[frameSlot], m_Fallback};
}

DisplaySurfaceGpuBinding DisplaySurfaceService::ResolveForUi(DisplaySurfaceToken token,
    const std::shared_ptr<const void>& lifetimePin, std::uint32_t frameSlot) noexcept
{
    const auto found = m_Resources.find(token);
    if (found == m_Resources.end() || frameSlot >= found->second->textures.size()
        || !lifetimePin || lifetimePin.get() != found->second->pin.get())
    {
        m_FallbackResolveCount.fetch_add(1, std::memory_order_relaxed);
        return FallbackBinding(frameSlot < m_FrameSlotCount ? frameSlot : 0);
    }
    const auto& resources = found->second;
    return {&resources->textures[frameSlot], &resources->views[frameSlot],
            const_cast<rhi::Sampler*>(&resources->sampler), resources->textureIds[frameSlot], resources};
}

void DisplaySurfaceService::DestroyResources(const ResourcesPtr& resources) noexcept
{
    if (!resources) return;
    for (const auto id : resources->textureIds) m_Arena.Destroy(id);
    resources->textureIds.clear();
}

void DisplaySurfaceService::CollectRetired() noexcept
{
    for (auto it = m_Resources.begin(); it != m_Resources.end();)
    {
        const auto& resources = it->second;
        if (resources->retired && resources.use_count() == 1 && resources->pin.use_count() == 1)
        {
            DestroyResources(resources);
            it = m_Resources.erase(it);
        }
        else ++it;
    }
}

void DisplaySurfaceService::ShutdownAfterGpuIdle() noexcept
{
    {
        std::lock_guard lock(m_PublicationMutex);
        m_Publications.clear();
    }
    m_Current.clear();
    for (const auto& [token, resources] : m_Resources)
    {
        (void)token;
        DestroyResources(resources);
    }
    m_Resources.clear();
    DestroyResources(m_Fallback);
    m_Fallback.reset();
    m_Initialized = false;
}
} // namespace Aether::Render
