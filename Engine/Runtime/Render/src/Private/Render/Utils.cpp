#include "Render/Utils.h"

namespace Aether
{
namespace Render
{
static VkIndexType MeshComponentTypeToVkIndexType(GpuMesh::IndexType type)
{
    switch (type)
    {
    case GpuMesh::IndexType::UINT16:
        return VK_INDEX_TYPE_UINT16;
    case GpuMesh::IndexType::UINT32:
        return VK_INDEX_TYPE_UINT32;
    default:
        assert(false && "Unsupported index type");
        return VK_INDEX_TYPE_UINT16; // Default to UINT16 to avoid compilation error
    }
}
static uint32_t GetIndexTypeByteSize(GpuMesh::IndexType type)
{
    switch (type)
    {
    case GpuMesh::IndexType::UINT16:
        return 2;
    case GpuMesh::IndexType::UINT32:
        return 4;
    default:
        assert(false && "Unsupported index type");
        return 0;
    }
}
void Utils::VkDrawMesh(vk::GraphicsCommandBuffer& cb, const GpuMesh& mesh,uint32_t instanceCnt)
{
    std::vector<VkBuffer> vertexBuffers(mesh.vertexBuffers.size());
    for (size_t i = 0; i < mesh.vertexBuffers.size(); i++)
    {
        vertexBuffers[i] = mesh.vertexBuffers[i].GetVk().GetHandle();
    }
    cb.BindVertexBuffers(vertexBuffers.data(), vertexBuffers.size());

    if (mesh.indexBuffer.has_value())
    {
        uint32_t indexCount = mesh.indexBuffer->GetSize() / GetIndexTypeByteSize(mesh.indexType);
        cb.BindIndexBuffer(mesh.indexBuffer->GetVk(),
                           MeshComponentTypeToVkIndexType(mesh.indexType),
                           0);
        cb.DrawIndexed(indexCount,instanceCnt);
    }
    else
    {
        cb.Draw(mesh.vertexCount,instanceCnt);
    }
}
void Utils::DrawMesh(rhi::CommandList& cb, const GpuMesh& mesh,uint32_t instanceCnt)
{
    if (Render::Config::RenderApi == Render::Api::Vulkan)
    {
        VkDrawMesh(cb.GetVk(), mesh,instanceCnt);
    }
    else
    {
        assert(false && "Not implemented");
    }
}
} // namespace Render
} // namespace Aether