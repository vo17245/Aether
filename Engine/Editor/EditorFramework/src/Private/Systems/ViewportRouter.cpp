#include <EditorFramework/Viewport.h>

#include <ProjectAsset/Types.h>

#include <cmath>
#include <limits>

namespace Aether::EditorFramework
{
std::expected<void, std::string> ViewportProviderRegistry::Register(ViewportProvider provider)
{
    if (m_Frozen) return std::unexpected(std::string("viewport provider registry is frozen"));
    if (!ProjectAssets::IsStableIdentifier(provider.id) || !ProjectAssets::IsStableIdentifier(provider.ownerFeature)
        || !provider.update)
        return std::unexpected(std::string("viewport provider requires stable IDs and an update callback"));
    if (m_Providers.contains(provider.id)) return std::unexpected(std::string("viewport provider is already registered"));
    const auto id = provider.id;
    m_Providers.emplace(id, std::move(provider));
    return {};
}

const ViewportProvider* ViewportProviderRegistry::Find(std::string_view id) const noexcept
{
    const auto it = m_Providers.find(std::string(id));
    return it == m_Providers.end() ? nullptr : &it->second;
}

bool ViewportRouter::Upsert(ViewportRequest request)
{
    if (!ProjectAssets::IsStableIdentifier(request.viewId) || !request.documentId.IsValid()
        || !request.worldInstanceId.IsValid() || !std::isfinite(request.logicalWidth)
        || !std::isfinite(request.logicalHeight) || !std::isfinite(request.pixelScale)
        || request.logicalWidth < 0.0 || request.logicalHeight < 0.0 || request.pixelScale <= 0.0)
        return false;
    const auto found = m_Views.find(request.viewId);
    if (found != m_Views.end())
    {
        const auto& old = found->second.request;
        const bool changed = old.documentId != request.documentId || old.worldInstanceId != request.worldInstanceId
            || old.purpose != request.purpose || old.providerId != request.providerId
            || old.logicalWidth != request.logicalWidth || old.logicalHeight != request.logicalHeight
            || old.pixelScale != request.pixelScale;
        if (changed && request.generation <= old.generation)
        {
            if (old.generation == std::numeric_limits<std::uint64_t>::max()) return false;
            request.generation = old.generation + 1;
        }
        if (request.generation < old.generation) return false;
        found->second = ViewportModel{std::move(request), ViewportStatus::MissingProvider, {}, {}};
    }
    else
    {
        if (request.generation == 0) request.generation = 1;
        const auto viewId = request.viewId;
        m_Views.emplace(viewId, ViewportModel{std::move(request), ViewportStatus::MissingProvider, {}, {}});
    }
    return true;
}

bool ViewportRouter::Remove(std::string_view viewId)
{
    if (m_FocusedView && *m_FocusedView == viewId) m_FocusedView.reset();
    return m_Views.erase(std::string(viewId)) != 0;
}

bool ViewportRouter::Focus(std::string_view viewId)
{
    if (!m_Views.contains(std::string(viewId))) return false;
    m_FocusedView = std::string(viewId);
    return true;
}

bool ViewportRouter::Suspend(std::string_view viewId, std::string reason)
{
    const auto found = m_Views.find(std::string(viewId));
    if (found == m_Views.end()) return false;
    found->second.status = ViewportStatus::Suspended;
    found->second.output = {};
    found->second.error = std::move(reason);
    return true;
}

void ViewportRouter::SetUiCapture(bool mouse, bool keyboard) noexcept
{
    m_CaptureMouse = mouse;
    m_CaptureKeyboard = keyboard;
    if (mouse) m_FocusedView.reset();
}

std::expected<ViewportModel, std::string> ViewportRouter::Update(std::string_view viewId,
    const ViewportProviderRegistry& providers)
{
    auto found = m_Views.find(std::string(viewId));
    if (found == m_Views.end()) return std::unexpected(std::string("viewport does not exist"));
    auto& model = found->second;
    if (model.request.logicalWidth == 0.0 || model.request.logicalHeight == 0.0)
    {
        model.status = ViewportStatus::Suspended;
        model.output = {};
        model.error.clear();
        return model;
    }
    const auto* provider = providers.Find(model.request.providerId);
    if (!provider)
    {
        model.status = ViewportStatus::MissingProvider;
        model.output = {};
        model.error = "no provider is registered for this viewport";
        return model;
    }
    try
    {
        auto output = provider->update(model.request);
        if (!output)
        {
            model.status = ViewportStatus::ProviderError;
            model.output = {};
            model.error = output.error();
            return model;
        }
        if (output->generation != model.request.generation
            || (!output->surface.IsValid() && !output->ownedPayload))
        {
            model.status = ViewportStatus::ProviderError;
            model.output = {};
            model.error = "provider output is stale or has no owned surface/payload";
            return model;
        }
        model.output = std::move(*output);
        model.status = ViewportStatus::Ready;
        model.error.clear();
    }
    catch (const std::exception& exception)
    {
        model.status = ViewportStatus::ProviderError;
        model.output = {};
        model.error = exception.what();
    }
    catch (...)
    {
        model.status = ViewportStatus::ProviderError;
        model.output = {};
        model.error = "viewport provider failed with an unknown error";
    }
    return model;
}

std::optional<RoutedViewportInput> ViewportRouter::RoutePointer(std::string_view viewId,
    const ViewportPointerInput& input) const
{
    const auto found = m_Views.find(std::string(viewId));
    if (found == m_Views.end() || !m_FocusedView || *m_FocusedView != viewId || m_CaptureMouse) return std::nullopt;
    const auto& request = found->second.request;
    const auto pixelWidth = request.logicalWidth * request.pixelScale;
    const auto pixelHeight = request.logicalHeight * request.pixelScale;
    if (!std::isfinite(input.pixelX) || !std::isfinite(input.pixelY) || pixelWidth <= 0.0 || pixelHeight <= 0.0
        || input.pixelX < 0.0 || input.pixelY < 0.0 || input.pixelX >= pixelWidth || input.pixelY >= pixelHeight)
        return std::nullopt;
    return RoutedViewportInput{request.viewId, request.worldInstanceId, request.generation,
        input.pixelX / pixelWidth, input.pixelY / pixelHeight, input.buttons, input.modifiers};
}

std::expected<ViewportPickRequest, std::string> ViewportRouter::RequestPick(std::string_view viewId,
    double pixelX, double pixelY, const ViewportProviderRegistry& providers)
{
    const auto found = m_Views.find(std::string(viewId));
    if (found == m_Views.end()) return std::unexpected(std::string("viewport does not exist"));
    if (!m_FocusedView || *m_FocusedView != viewId || m_CaptureMouse)
        return std::unexpected(std::string("viewport does not own pointer focus"));
    const auto& request = found->second.request;
    if (!providers.Find(request.providerId) || !providers.Find(request.providerId)->pick)
        return std::unexpected(std::string("viewport provider does not support picking"));
    const double width = request.logicalWidth * request.pixelScale;
    const double height = request.logicalHeight * request.pixelScale;
    if (!std::isfinite(pixelX) || !std::isfinite(pixelY) || width <= 0.0 || height <= 0.0
        || pixelX < 0.0 || pixelY < 0.0 || pixelX >= width || pixelY >= height)
        return std::unexpected(std::string("pick coordinates are outside the viewport"));
    if (m_NextPickId == 0 || m_NextPickId == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected(std::string("viewport pick request IDs are exhausted"));
    return ViewportPickRequest{m_NextPickId++, request.viewId, request.documentId, request.worldInstanceId,
        request.generation, pixelX / width, pixelY / height};
}

std::expected<ViewportPickResult, std::string> ViewportRouter::ExecutePick(const ViewportPickRequest& request,
    const ViewportProviderRegistry& providers) const
{
    const auto found = m_Views.find(request.viewId);
    if (found == m_Views.end() || found->second.request.generation != request.generation
        || found->second.request.worldInstanceId != request.worldInstanceId
        || found->second.request.documentId != request.documentId)
        return std::unexpected(std::string("pick request targets a stale viewport generation"));
    const auto* provider = providers.Find(found->second.request.providerId);
    if (!provider || !provider->pick) return std::unexpected(std::string("viewport provider does not support picking"));
    auto picked = provider->pick(request);
    if (!picked) return std::unexpected(picked.error());
    return AcceptPickResult({request, std::move(*picked)});
}

std::expected<ViewportPickResult, std::string> ViewportRouter::AcceptPickResult(ViewportPickResult result) const
{
    const auto found = m_Views.find(result.request.viewId);
    if (found == m_Views.end() || found->second.request.generation != result.request.generation
        || found->second.request.worldInstanceId != result.request.worldInstanceId
        || found->second.request.documentId != result.request.documentId)
        return std::unexpected(std::string("pick result belongs to a stale viewport generation"));
    if (result.entity && !result.entity->IsValid())
        return std::unexpected(std::string("pick result contains an invalid persistent entity ID"));
    return result;
}

const ViewportModel* ViewportRouter::Find(std::string_view viewId) const noexcept
{
    const auto it = m_Views.find(std::string(viewId));
    return it == m_Views.end() ? nullptr : &it->second;
}
}
