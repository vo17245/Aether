#pragma once
#include "../../Render/Renderer.h"
#include <Scene/Scene.h>
#include <Window/Event.h>
namespace Aether::UI
{
class SystemI
{
public:
    virtual void OnUpdate(float sec, Scene& scene) {}
    virtual void OnRender(DeviceCommandBufferView commandBuffer,
                          DeviceFrameBufferView frameBuffer,
                          Vec2f screenSize,
                          Scene& scene) {}
    virtual void OnEvent(Event& event, Scene& scene) {}
    
    virtual ~SystemI() = default;
};
}