#include "VkMesh.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/GlobalRenderContext.h"
namespace Aether
{
void VkMesh::Update(const Mesh& mesh)
{
    auto& commandPool = vk::GRC::GetGraphicsCommandPool();
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
#ifdef AETHER_RUNTIME_CHECK
        if (indexResource.type != mesh.accessors[mesh.primitive.index.value()].componentType)
        {
            assert(false && "index type mismatch");
        }
#endif
    }
}
bool VkMesh::CreateBuffers(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
{
    // create staging buffer
    uint32_t maxBufferSize = 0;
    for (const auto& bufferView : Mesh.bufferViews)
    {
        maxBufferSize = std::max(maxBufferSize, bufferView.size);
    }
    auto stagingBufferOpt = vk::Buffer::CreateForStaging(maxBufferSize);
    if (!stagingBufferOpt.has_value())
    {
        assert(false && "failed to create staging buffer");
        return false;
    }
    auto stagingBuffer = std::move(stagingBufferOpt.value());
    // create data for each bufferview and copy data
    for (const auto& bufferView : Mesh.bufferViews)
    {
        const auto& buffer = Mesh.buffers[bufferView.buffer];
        const size_t size = bufferView.size;
        const size_t offset = bufferView.offset;
#if AETHER_RUNTIME_CHECK
        if (size + offset > buffer.size())
        {
            assert(false && "buffer overflow");
            return false;
        }
#endif
        if (bufferView.target == Mesh::Target::Index)
        {
            auto vkBufferOpt = vk::Buffer::CreateForIndex(size);
            if (!vkBufferOpt.has_value())
            {
                assert(false && "failed to create index buffer");
                return false;
            }
            auto vkBuffer = std::move(vkBufferOpt.value());
            // copy data
            stagingBuffer.SetData(buffer.data() + offset, size);
            vk::Buffer::SyncCopy(commandPool, stagingBuffer, vkBuffer, size);
            m_Buffers.emplace_back(std::move(vkBuffer));
        }
        else
        {
            auto bufferOpt = vk::Buffer::CreateForVertex(bufferView.size);
            if (!bufferOpt.has_value())
            {
                assert(false && "failed to create vertex buffer");
                return false;
            }
            // copy data
            stagingBuffer.SetData(buffer.data() + offset, size);
            vk::Buffer::SyncCopy(commandPool, stagingBuffer, bufferOpt.value(), size);
            m_Buffers.emplace_back(std::move(bufferOpt.value()));
        }
        m_BufferTargets.push_back(bufferView.target);
    }
    return true;
}
} // namespace Aether