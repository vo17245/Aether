#pragma once
#include "Resource/AccessId.h"
#include <Render/RenderApi.h>
#include "TaskBase.h"
namespace Aether::RenderGraph
{
    struct ImageLayoutTransitionTask:public TaskBase
    {
        ImageLayoutTransitionTask() : TaskBase(TaskType::ImageLayoutTransitionTask) {}
        AccessId<DeviceTexture> texture;
        DeviceImageLayout oldLayout;
        DeviceImageLayout newLayout;
    };
}