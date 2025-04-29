#pragma once
#include "../../Render/Renderer.h"
#include <Scene/Scene.h>
namespace Aether::UI
{
class SystemI
{
public:
    virtual void OnUpdate(float sec, Scene& scene) = 0;
    virtual void OnRender(DeviceCommandBufferView commandBuffer,
                          DeviceRenderPassView renderPass,
                          DeviceFrameBufferView frameBuffer,
                          Vec2f screenSize,
                          Scene& scene) = 0;
    virtual ~SystemI() = default;
};
}