#pragma once
#include "Resource/AccessId.h"
#include <Render/RenderApi.h>
#include "TaskBase.h"
#include "Resource/DeviceTexture.h"
#include "Resource/ResourceAccessor.h"
namespace Aether::RenderGraph
{
    struct ImageLayoutTransitionTask:public TaskBase
    {
        ImageLayoutTransitionTask() : TaskBase(TaskType::ImageLayoutTransitionTask) {}
        AccessId<DeviceTexture> texture;
        DeviceImageLayout oldLayout;
        DeviceImageLayout newLayout;
        void Execute(DeviceCommandBuffer& commandBuffer, ResourceAccessor& resourceAccessor) 
        {
            auto* textureActual= resourceAccessor.GetResource(texture);
            textureActual->AsyncTransitionLayout(oldLayout, newLayout, commandBuffer);
        }
    };
}