#pragma once
#include "../Vulkan/GraphicsCommandBuffer.h"
#include <variant>
namespace Aether
{
using DeviceCommandBuffer = std::variant<std::monostate, vk::GraphicsCommandBuffer>;
}