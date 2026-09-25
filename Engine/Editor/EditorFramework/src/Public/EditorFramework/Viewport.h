#pragma once

#include <Core/DisplaySurfaceToken.h>
#include <GameFeature/Types.h>

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Aether::EditorFramework
{
enum class ViewportPurpose { View, Play, AssetPreview };
enum class ViewportStatus { Ready, MissingProvider, ProviderError, Suspended };

struct ViewportRequest
{
    std::string viewId;
    GameFeatures::DocumentId documentId;
    GameFeatures::WorldInstanceId worldInstanceId;
    ViewportPurpose purpose = ViewportPurpose::View;
    std::string providerId;
    double logicalWidth = 0.0;
    double logicalHeight = 0.0;
    double pixelScale = 1.0;
    std::uint64_t generation = 1;
};

struct ViewportOutput
{
    std::uint64_t generation = 0;
    DisplaySurfaceToken surface;
    std::shared_ptr<const std::vector<std::byte>> ownedPayload;
    std::string diagnostic;
};

struct ViewportPickRequest
{
    std::uint64_t requestId = 0;
    std::string viewId;
    GameFeatures::DocumentId documentId;
    GameFeatures::WorldInstanceId worldInstanceId;
    std::uint64_t generation = 0;
    double normalizedX = 0.0;
    double normalizedY = 0.0;
};

struct ViewportPickResult
{
    ViewportPickRequest request;
    std::optional<GameFeatures::PersistentEntityId> entity;
};

struct ViewportProvider
{
    std::string id;
    GameFeatures::FeatureId ownerFeature;
    std::function<std::expected<ViewportOutput, std::string>(const ViewportRequest&)> update;
    std::function<std::expected<std::optional<GameFeatures::PersistentEntityId>, std::string>(
        const ViewportPickRequest&)> pick;
};

class ViewportProviderRegistry
{
public:
    std::expected<void, std::string> Register(ViewportProvider provider);
    void Freeze() noexcept { m_Frozen = true; }
    const ViewportProvider* Find(std::string_view id) const noexcept;
private:
    bool m_Frozen = false;
    std::unordered_map<std::string, ViewportProvider> m_Providers;
};

struct ViewportModel
{
    ViewportRequest request;
    ViewportStatus status = ViewportStatus::MissingProvider;
    ViewportOutput output;
    std::string error;
};

struct ViewportPointerInput
{
    double pixelX = 0.0;
    double pixelY = 0.0;
    std::uint32_t buttons = 0;
    std::uint32_t modifiers = 0;
};

struct RoutedViewportInput
{
    std::string viewId;
    GameFeatures::WorldInstanceId worldInstanceId;
    std::uint64_t generation = 0;
    double normalizedX = 0.0;
    double normalizedY = 0.0;
    std::uint32_t buttons = 0;
    std::uint32_t modifiers = 0;
};

class ViewportRouter
{
public:
    bool Upsert(ViewportRequest request);
    bool Remove(std::string_view viewId);
    bool Focus(std::string_view viewId);
    void SetUiCapture(bool mouse, bool keyboard) noexcept;
    std::expected<ViewportModel, std::string> Update(std::string_view viewId,
        const ViewportProviderRegistry& providers);
    std::optional<RoutedViewportInput> RoutePointer(std::string_view viewId,
        const ViewportPointerInput& input) const;
    std::expected<ViewportPickRequest, std::string> RequestPick(std::string_view viewId,
        double pixelX, double pixelY, const ViewportProviderRegistry& providers);
    std::expected<ViewportPickResult, std::string> ExecutePick(const ViewportPickRequest& request,
        const ViewportProviderRegistry& providers) const;
    std::expected<ViewportPickResult, std::string> AcceptPickResult(ViewportPickResult result) const;
    const ViewportModel* Find(std::string_view viewId) const noexcept;
    std::optional<std::string> FocusedView() const { return m_FocusedView; }
private:
    std::uint64_t m_NextPickId = 1;
    std::optional<std::string> m_FocusedView;
    bool m_CaptureMouse = false;
    bool m_CaptureKeyboard = false;
    std::unordered_map<std::string, ViewportModel> m_Views;
};
}
