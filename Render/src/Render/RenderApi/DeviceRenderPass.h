#pragma once
#include "../Vulkan/RenderPass.h"
#include <variant>
namespace Aether
{
    using DeviceRenderPass=std::variant<std::monostate,vk::RenderPass>;
}