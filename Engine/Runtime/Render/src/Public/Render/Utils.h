#pragma once

#include <Render/RHI.h>
#include <Render/Mesh/GpuMesh.h>
namespace Aether {
namespace Render {
class Utils
{
public:
    /**
     * @brief 用于向command buffer 录制时使用
     * 在bind pipeline之后调用来绘制网格
     */
    static void VkDrawMesh(vk::GraphicsCommandBuffer& cb, const GpuMesh& mesh,uint32_t instanceCnt=1);
    static void DrawMesh(rhi::CommandList& cb, const GpuMesh& mesh,uint32_t instanceCnt=1);
    static void SyncUploadTexture2D(const rhi::StagingBuffer& src,rhi::Texture2D& dst);
};
}
} // namespace Aether::Render