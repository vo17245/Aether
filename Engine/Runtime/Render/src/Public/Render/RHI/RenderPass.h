#pragma once
#include "Attachment.h"
#include "Config.h"
namespace Aether::rhi
{
    struct RenderPass
    {
        std::vector<ColorAttachment> colorAttachments;
        std::optional<DepthAttachment> depthAttachment;
    };
}