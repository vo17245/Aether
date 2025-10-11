#include "Core/Base.h"
#include "FrameBuffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "Render/Vulkan/Traits.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <type_traits>
#include "Buffer.h"
#include "Fence.h"
#include <format>
#include "GraphicsCommandPool.h"
#include "GraphicsCommandBuffer.h"
#include "GlobalRenderContext.h"
namespace Aether::vk {

std::optional<GraphicsCommandBuffer> GraphicsCommandBuffer::Create(const GraphicsCommandPool& pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool.GetHandle();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(GRC::GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        return std::nullopt;
    }

    return GraphicsCommandBuffer(commandBuffer, &pool);
}
Scope<GraphicsCommandBuffer> GraphicsCommandBuffer::CreateScope(const GraphicsCommandPool& pool)
{
    auto opt = Create(pool);
    if (!opt.has_value())
    {
        return nullptr;
    }
    return Aether::CreateScope<GraphicsCommandBuffer>(std::move(opt.value()));
}
GraphicsCommandBuffer::~GraphicsCommandBuffer()
{
    if (m_CommandBuffer != VK_NULL_HANDLE)
        vkFreeCommandBuffers(GRC::GetDevice(), m_CommandPool->GetHandle(), 1, &m_CommandBuffer);
}
VkCommandBuffer GraphicsCommandBuffer::GetHandle() const
{
    return m_CommandBuffer;
}

GraphicsCommandBuffer::GraphicsCommandBuffer(GraphicsCommandBuffer&& other) noexcept
{
    m_CommandBuffer = other.m_CommandBuffer;
    other.m_CommandBuffer = VK_NULL_HANDLE;
    m_CommandPool = other.m_CommandPool;
    other.m_CommandPool = nullptr;
}
GraphicsCommandBuffer& GraphicsCommandBuffer::operator=(GraphicsCommandBuffer&& other) noexcept
{
    if (this != &other)
    {
        if (m_CommandBuffer != VK_NULL_HANDLE)
            vkFreeCommandBuffers(GRC::GetDevice(), m_CommandPool->GetHandle(), 1, &m_CommandBuffer);
        m_CommandBuffer = other.m_CommandBuffer;
        other.m_CommandBuffer = VK_NULL_HANDLE;

        m_CommandPool = other.m_CommandPool;
        other.m_CommandPool = nullptr;
    }
    return *this;
}
void GraphicsCommandBuffer::Reset()
{
    vkResetCommandBuffer(m_CommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
}
VkResult GraphicsCommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
}
VkResult GraphicsCommandBuffer::BeginSingleTime()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    return (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
}

void GraphicsCommandBuffer::SetViewport(float x, float y, float width, float height)
{
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
}
void GraphicsCommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    VkRect2D scissor{};
    scissor.offset = {x, y};
    scissor.extent = {width, height};
    vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
}

void GraphicsCommandBuffer::BindPipeline(const GraphicsPipeline& pipeline)
{
    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetHandle());
}

VkResult GraphicsCommandBuffer::End()
{
    return (vkEndCommandBuffer(m_CommandBuffer));
}
void GraphicsCommandBuffer::Draw(uint32_t vertexCnt,uint32_t instanceCnt)
{
    vkCmdDraw(m_CommandBuffer, vertexCnt, instanceCnt, 0, 0);
}
void GraphicsCommandBuffer::DrawIndexed(uint32_t indexCnt,uint32_t instanceCnt)
{
    vkCmdDrawIndexed(m_CommandBuffer, indexCnt, instanceCnt, 0, 0, 0);
}
void GraphicsCommandBuffer::BeginRenderPass(const RenderPass& renderPass, const FrameBuffer& framebuffer, const Vec4f& clearValue)
{
    static_assert(sizeof(Vec4f) == sizeof(VkClearValue), "Vec4f size must be equal to VkClearValue size");
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.GetHandle();
    renderPassInfo.framebuffer = framebuffer.GetHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = framebuffer.GetSize();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = (VkClearValue*)&clearValue;

    return vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void GraphicsCommandBuffer::BeginRenderPass(const RenderPass& renderPass, const FrameBuffer& framebuffer)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.GetHandle();
    renderPassInfo.framebuffer = framebuffer.GetHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = framebuffer.GetSize();

    return vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GraphicsCommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(m_CommandBuffer);
}

GraphicsCommandBuffer::SubmitBuilder GraphicsCommandBuffer::BeginSubmit()
{
    return SubmitBuilder(*this);
}
VkResult GraphicsCommandBuffer::Submit(uint32_t waitCnt, VkSemaphore* waitSemaphores, VkPipelineStageFlags* waitStages, uint32_t signalCnt, VkSemaphore* signalSemaphores, VkFence fence)

{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = waitCnt;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffer;

    submitInfo.signalSemaphoreCount = signalCnt;
    submitInfo.pSignalSemaphores = signalSemaphores;

    return (vkQueueSubmit(GRC::GetGraphicsQueue().GetHandle(), 1, &submitInfo, fence));
}
void GraphicsCommandBuffer::BindVertexBuffer(Buffer& buffer, uint32_t firstBinding)
{
    VkBuffer vertexBuffers[] = {buffer.GetHandle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, 1, vertexBuffers, offsets);
}

void GraphicsCommandBuffer::BindVertexBuffers(const VkBuffer* buffers, uint32_t bufferCnt, uint32_t firstBinding)
{
    std::vector<VkDeviceSize> offsets(bufferCnt);

    vkCmdBindVertexBuffers(m_CommandBuffer,
                           firstBinding,
                           bufferCnt,
                           buffers,
                           offsets.data());
}
void GraphicsCommandBuffer::BindVertexBuffers(const VkBuffer* buffers, uint32_t bufferCnt, uint32_t firstBinding,
                                              const VkDeviceSize* offsets)
{
    vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, bufferCnt, buffers, offsets);
}
void GraphicsCommandBuffer::BindVertexBuffers(const std::vector<VkBuffer>& buffers, uint32_t firstBinding)
{
    BindVertexBuffers(buffers.data(), buffers.size(), firstBinding);
}

void GraphicsCommandBuffer::BindIndexBuffer(Buffer& buffer, VkIndexType type, uint32_t offset)
{
    vkCmdBindIndexBuffer(m_CommandBuffer, buffer.GetHandle(), offset, type);
}
void GraphicsCommandBuffer::BindDescriptorSet(DescriptorSet& descriptorSet, PipelineLayout& pipelineLayout, uint32_t setIndex)
{
    const VkDescriptorSet descriptorSets[] = {descriptorSet.GetHandle()};
    vkCmdBindDescriptorSets(m_CommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout.GetHandle(),
                            setIndex,
                            1,
                            descriptorSets,
                            0,
                            nullptr);
}
void GraphicsCommandBuffer::UpdatePushConstants(const void* data, uint32_t size, uint32_t offset, PipelineLayout& pipelineLayout, vk::ShaderStage stage)
{
    vkCmdPushConstants(m_CommandBuffer,
                       pipelineLayout.GetHandle(),
                       static_cast<VkShaderStageFlags>(stage),
                       offset,
                       size,
                       data);
}
void GraphicsCommandBuffer::UpdatePushConstantsOnVertexStage(const void* data, uint32_t size, uint32_t offset, PipelineLayout& pipelineLayout)
{
    vkCmdPushConstants(m_CommandBuffer,
                       pipelineLayout.GetHandle(),
                       VK_SHADER_STAGE_VERTEX_BIT,
                       offset, size, data);
}
void GraphicsCommandBuffer::UpdatePushConstantsOnFragmentStage(const void* data, uint32_t size, uint32_t offset, PipelineLayout& pipelineLayout)
{
    vkCmdPushConstants(m_CommandBuffer,
                       pipelineLayout.GetHandle(),
                       VK_SHADER_STAGE_FRAGMENT_BIT,
                       offset, size, data);
}

GraphicsCommandBuffer::GraphicsCommandBuffer(VkCommandBuffer commandBuffer, const GraphicsCommandPool* commandPool) :
    m_CommandBuffer(commandBuffer), m_CommandPool(commandPool)
{
}
void GraphicsCommandBuffer::BeginRenderPass(const RenderPass& renderpass,const FrameBuffer& framebuffer,std::span<VkClearValue> clearValues)
{
 VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderpass.GetHandle();
    renderPassInfo.framebuffer = framebuffer.GetHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = framebuffer.GetSize();
    renderPassInfo.clearValueCount= clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    return vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

} // namespace Aether::vk