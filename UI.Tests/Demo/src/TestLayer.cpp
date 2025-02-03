#include "TestLayer.h"
#include "Render/Vulkan/RenderPass.h"
#include "vulkan/vulkan_core.h"

using namespace Aether;
bool TestLayer::CreateRenderPass()
{
    m_RenderPass=vk::RenderPass::CreateDefault().value();
    return true;
}