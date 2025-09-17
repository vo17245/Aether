#pragma once

#include <Render/RenderApi.h>
namespace Aether {
namespace Render {
class Utils
{
public:
    /**
     * @brief 用于向command buffer 录制时使用
     * 在bind pipeline之后调用来绘制网格
     */
    static void VkDrawMesh(vk::GraphicsCommandBuffer& cb, VkMesh& mesh);
    static void DrawMesh(DeviceCommandBuffer& cb, DeviceMesh& mesh);
};
}
} // namespace Aether::Render