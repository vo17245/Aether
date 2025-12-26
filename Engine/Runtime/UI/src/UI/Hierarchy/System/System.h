#pragma once
#include "../../Render/Renderer.h"
#include <World/World.h>
#include <Window/Event.h>
namespace Aether::UI
{
class SystemI
{
public:
    virtual void OnUpdate(float sec, World& scene) {}
    virtual void OnRender(DeviceCommandBufferView commandBuffer,
                          DeviceFrameBufferView frameBuffer,
                          Vec2f screenSize,
                          World& scene) {}
    virtual void OnEvent(Event& event, World& scene) {}
    
    virtual ~SystemI() = default;
};
}