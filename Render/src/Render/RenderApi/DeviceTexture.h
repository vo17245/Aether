#pragma once
#include "../Vulkan/Texture2D.h"
#include <variant>
namespace Aether
{
using DeviceTexture=std::variant<std::monostate,vk::Texture2D>;
}