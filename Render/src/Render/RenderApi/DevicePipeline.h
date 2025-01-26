#pragma once
#include "../Vulkan/Pipeline.h"
#include "Render/Vulkan/Pipeline.h"
#include <variant>
namespace Aether
{
    using DevicePipeline=std::variant<std::monostate,vk::GraphicsPipeline>;
}