#pragma once
#include "Render/Scene/Camera2D.h"
#include <UI/Render/Renderer.h>
#include <Core/Core.h>

namespace Aether::UI
{
class DebugUILayerResource
{
public:
    DebugUILayerResource(DebugUILayerResource&&) = default;
    DebugUILayerResource() = default;
    DebugUILayerResource(const DebugUILayerResource&) = delete;
    DebugUILayerResource& operator=(const DebugUILayerResource&) = delete;
    DebugUILayerResource& operator=(DebugUILayerResource&&) = default;

    Scope<UI::Renderer> renderer;
    UI::RenderResource renderResource;
    Camera2D camera;

    static std::optional<DebugUILayerResource> Create(const Vec2i& screenSize)
    {
        auto res = DebugUILayerResource();
        res.CreateRenderResource();
        res.CreateRenderer();
        res.InitCamera(screenSize.cast<float>());
        return res;
    }
    bool ResizeHierarchyFrameBuffer(const Vec2i& screenSize)
    {
        camera.screenSize = screenSize.cast<float>();
        return true;
    }

private:
    bool CreateRenderResource()
    {
        auto stagingBuffer = UI::DynamicStagingBuffer(1024);
        renderResource.m_StagingBuffer = CreateRef<UI::DynamicStagingBuffer>(std::move(stagingBuffer));
        return true;
    }
    bool CreateRenderer()
    {
        auto rendererEx = UI::Renderer::Create(renderResource);
        if (!rendererEx)
        {
            return false;
        }
        renderer = CreateScope<UI::Renderer>(std::move(rendererEx.value()));
        return true;
    }
    void InitCamera(const Vec2f& screenSize)
    {
        camera.screenSize = screenSize;
        camera.target = Vec2f(screenSize.x() / 2, screenSize.y() / 2);
        camera.offset = Vec2f(0, 0);
        camera.near = 0.0f;
        camera.far = 10000.0f;
        camera.zoom = 1.0f;
        camera.rotation = 0.0f;
    }
};
} // namespace Aether::UI
