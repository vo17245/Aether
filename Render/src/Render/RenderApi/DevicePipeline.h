#pragma once
#include "../Vulkan/Pipeline.h"
#include "Render/Vulkan/Pipeline.h"
#include <variant>
namespace Aether
{
    using DevicePipelineLayout=std::variant<std::monostate,vk::PipelineLayout>;
    using DevicePipeline=std::variant<std::monostate,vk::GraphicsPipeline>;
}