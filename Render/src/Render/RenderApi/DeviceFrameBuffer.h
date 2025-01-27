#pragma once
#include "../Vulkan/FrameBuffer.h"
#include <variant>
namespace Aether
{
using DeviceFrameBuffer=std::variant<std::monostate,vk::FrameBuffer>;
}