#include "Utils.h"

namespace Aether
{
namespace Render
{

void Utils::VkDrawMesh(vk::GraphicsCommandBuffer& cb, VkMesh& mesh)
{
    cb.BindVertexBuffers(mesh.PackPrimitiveVertexBufferHandles().data(),
                         mesh.GetPrimitive().vertexResource.buffers.size());

    if (mesh.GetPrimitive().indexResource)
    {
        cb.BindIndexBuffer(mesh.GetBuffers()[mesh.GetPrimitive().indexResource->buffer],
                           MeshComponentTypeToVkIndexType(mesh.GetPrimitive().indexResource->type),
                           mesh.GetPrimitive().indexResource->offset);
        cb.DrawIndexed(mesh.GetPrimitive().indexResource->count);
    }
    else
    {
        cb.Draw(mesh.GetVertexCount());
    }
}
void Utils::DrawMesh(DeviceCommandBuffer& cb, DeviceMesh& mesh)
{
    if (Render::Config::RenderApi == Render::Api::Vulkan)
    {
        VkDrawMesh(cb.GetVk(), mesh.GetVk());
    }
    else
    {
        assert(false && "Not implemented");
    }
}
} // namespace Render
} // namespace Aether