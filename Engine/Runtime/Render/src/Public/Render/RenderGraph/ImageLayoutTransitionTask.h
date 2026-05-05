#pragma once
#include "Resource/AccessId.h"
#include "TaskBase.h"
#include "Resource/ResourceAccessor.h"
namespace Aether::RenderGraph
{
    struct ImageLayoutTransitionTask:public TaskBase
    {
        ImageLayoutTransitionTask() : TaskBase(TaskType::ImageLayoutTransitionTask) {}
        AccessId<rhi::Texture2D> texture;
        rhi::TextureLayout oldLayout;
        rhi::TextureLayout newLayout;
        void Execute(rhi::CommandList& commandBuffer, ResourceAccessor& resourceAccessor) 
        {
            auto* textureActual= resourceAccessor.GetResource(texture);
            commandBuffer.TextureLayoutTransition(*textureActual, oldLayout, newLayout);
        }
    };
}