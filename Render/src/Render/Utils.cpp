#include "Utils.h"

namespace Aether {
namespace Render {

void Utils::DrawGrid(vk::GraphicsCommandBuffer& cb, VkGrid& mesh)
{
    cb.BindVertexBuffers(mesh.PackPrimitiveVertexBufferHandles().data(),
                         mesh.GetPrimitive().vertexResource.buffers.size());

    if (mesh.GetPrimitive().indexResource)
    {
        cb.BindIndexBuffer(mesh.GetBuffers()[mesh.GetPrimitive().indexResource->buffer],
                           GridComponentTypeToVkIndexType(mesh.GetPrimitive().indexResource->type),
                           mesh.GetPrimitive().indexResource->offset);
        cb.DrawIndexed(mesh.GetPrimitive().indexResource->count);
    }
    else
    {
        cb.Draw(mesh.GetVertexCount());
    }
}
}
} // namespace Aether::Render