#include "VkMesh.h"
#include "Render/Vulkan/Buffer.h"
namespace Aether {
void VkMesh::Update(vk::GraphicsCommandPool& commandPool, const Mesh& mesh)
{
    // 更新buffer
    for (size_t i = 0; i < mesh.bufferViews.size(); i++)
    {
        auto& vkBuffer = m_Buffers[i];
        auto& bufferView = mesh.bufferViews[i];
        // 检查是否需要创建新的vk::Buffer

        if (vkBuffer.GetSize() < bufferView.size)
        {
            if (bufferView.target == Mesh::Target::Vertex)
            {
                vkBuffer = std::move(vk::Buffer::CreateForVertex(bufferView.size).value());
            }
            else if (bufferView.target == Mesh::Target::Index)
            {
                vkBuffer = std::move(vk::Buffer::CreateForIndex(bufferView.size).value());
            }
            else
            {
                assert(false && "unknown target");
            }
        }
        // 更新数据
        auto stagingBuffer = vk::Buffer::CreateForStaging(bufferView.size).value();
        stagingBuffer.SetData(mesh.buffers[bufferView.buffer].data() + bufferView.offset, bufferView.size);
        vk::Buffer::SyncCopy(commandPool, stagingBuffer, vkBuffer, bufferView.size);
    }
    if (m_Primitive.indexResource.has_value())

    {
        auto& indexResource = m_Primitive.indexResource.value();
        auto& accessor = mesh.accessors[mesh.primitive.index.value()];
        auto& bufferView = mesh.bufferViews[accessor.bufferView];
        indexResource.count = accessor.count;
        indexResource.offset = bufferView.offset;
#ifdef ENABLE_VALIDATION
        if (indexResource.type != mesh.accessors[mesh.primitive.index.value()].componentType)
        {
            ASSERT(false && "index type mismatch");
        }
#endif
    }
}
} // namespace Kamui