#include <Render/RHI/CommandList.h>
#include <Render/RHI/Backend/Vulkan/GraphicsCommandBuffer.h>
#include <Render/RHI/Backend/Vulkan/Transfer.h>

namespace Aether::rhi
{
// clang-format off
static constexpr inline VkCompareOp RHICompareOpToVk(CompareOp op)
{
    switch (op) 
    {
    case CompareOp::NEVER: return VK_COMPARE_OP_NEVER;
    case CompareOp::LESS: return VK_COMPARE_OP_LESS;
    case CompareOp::EQUAL: return VK_COMPARE_OP_EQUAL;
    case CompareOp::LESS_OR_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::GREATER: return VK_COMPARE_OP_GREATER;
    case CompareOp::NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::GREATER_OR_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::ALWAYS: return VK_COMPARE_OP_ALWAYS;
    default:
        assert(false && "unknown CompareOp");
        return VK_COMPARE_OP_ALWAYS;
    }
}
// clang-format on
void CommandList::SetViewport(float x, float y, float width, float height)
{
    if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
    {
        auto& cb = std::get<vk::GraphicsCommandBuffer>(m_Data);
        cb.SetViewport(x, y, width, height);
    }
    else
    {
        assert(false && "unsupported command buffer type");
    }
}
void CommandList::SetScissor(float x, float y, float width, float height)
{
    if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
    {
        auto& cb = std::get<vk::GraphicsCommandBuffer>(m_Data);
        cb.SetScissor(x, y, width, height);
    }
    else
    {
        assert(false && "unsupported command buffer type");
    }
}
void CommandList::BindPipeline(DevicePipeline& pipeline)
{
    if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
    {
        auto& cb = std::get<vk::GraphicsCommandBuffer>(m_Data);
        cb.BindPipeline(pipeline.GetVk());
    }
    else
    {
        assert(false && "unsupported command buffer type");
    }
}
/**
 * @brief record a buffer copy command
 */
void CommandList::UploadVertexBuffer(StagingBuffer& src, VertexBuffer& dst, size_t size, size_t srcOffset,
                                     size_t dstOffset)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        vk::AsyncCopyBuffer(std::get<vk::GraphicsCommandBuffer>(m_Data), std::get<vk::Buffer>(src.m_Buffer),
                            std::get<vk::Buffer>(dst.m_Buffer), size, srcOffset, dstOffset);
    }
    break;
    default:
        assert(false && "unsupported api");
        break;
    }
}
void CommandList::UploadIndexBuffer(StagingBuffer& src, IndexBuffer& dst, size_t size, size_t srcOffset,
                                    size_t dstOffset)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        vk::AsyncCopyBuffer(std::get<vk::GraphicsCommandBuffer>(m_Data), std::get<vk::Buffer>(src.m_Buffer),
                            std::get<vk::Buffer>(dst.m_Buffer), size, srcOffset, dstOffset);
    }
    break;
    default:
        assert(false && "unsupported api");
        break;
    }
}
void CommandList::UploadUniformBuffer(StagingBuffer& src, UniformBuffer& dst, size_t size, size_t srcOffset,
                                      size_t dstOffset)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        vk::AsyncCopyBuffer(std::get<vk::GraphicsCommandBuffer>(m_Data), std::get<vk::Buffer>(src.m_Buffer),
                            std::get<vk::Buffer>(dst.m_Buffer), size, srcOffset, dstOffset);
    }
    break;
    default:
        assert(false && "unsupported api");
        break;
    }
}
void CommandList::UploadUniformBuffer(StagingBuffer& src, RWStructuredBuffer& dst, size_t size, size_t srcOffset,
                                      size_t dstOffset)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        vk::AsyncCopyBuffer(std::get<vk::GraphicsCommandBuffer>(m_Data), std::get<vk::Buffer>(src.m_Buffer),
                            std::get<vk::Buffer>(dst.m_Buffer), size, srcOffset, dstOffset);
    }
    break;
    default:
        assert(false && "unsupported api");
        break;
    }
}
void CommandList::SetDepthCompareOp(CompareOp op)
{
    if (std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
    {
        auto& cb = std::get<vk::GraphicsCommandBuffer>(m_Data);
        cb.SetDepthCompareOp(RHICompareOpToVk(op));
    }
    else
    {
        assert(false && "unsupported command buffer type");
    }
}

} // namespace Aether::rhi
