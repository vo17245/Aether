#include "Render/Utils.h"

namespace Aether
{
namespace Render
{

void Utils::VkDrawMesh(vk::GraphicsCommandBuffer& cb, const GpuMesh& mesh,uint32_t instanceCnt)
{
    cb.BindVertexBuffers(mesh.PackPrimitiveVertexBufferHandles().data(),
                         mesh.GetPrimitive().vertexResource.buffers.size());

    if (mesh.GetPrimitive().indexResource)
    {
        cb.BindIndexBuffer(mesh.GetBuffers()[mesh.GetPrimitive().indexResource->buffer],
                           MeshComponentTypeToVkIndexType(mesh.GetPrimitive().indexResource->type),
                           mesh.GetPrimitive().indexResource->offset);
        cb.DrawIndexed(mesh.GetPrimitive().indexResource->count,instanceCnt);
    }
    else
    {
        cb.Draw(mesh.GetVertexCount(),instanceCnt);
    }
}
void Utils::DrawMesh(rhi::CommandList& cb, const GpuMesh& mesh,uint32_t instanceCnt)
{
    if (Render::Config::RenderApi == Render::Api::Vulkan)
    {
        VkDrawMesh(cb.GetVk(), mesh.GetVk(),instanceCnt);
    }
    else
    {
        assert(false && "Not implemented");
    }
}
} // namespace Render
} // namespace Aether