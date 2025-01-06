#pragma once
#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/GraphicsCommandPool.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderContext.h"
#include "Render/Vulkan/ShaderModule.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "Render/Mesh/VkGrid.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
namespace Aether {
namespace Render {
class Utils
{
public:
    /**
     * @brief 用于向command buffer 录制时使用
     * 在bind pipeline之后调用来绘制网格
     */
    static void DrawGrid(vk::GraphicsCommandBuffer& cb, VkGrid& mesh);
};
}
} // namespace Aether::Render