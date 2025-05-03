#pragma once
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
#include "Traits.h"
#include "GraphicsCommandPool.h"
namespace Aether {
namespace vk {
template <typename T>
struct IndexType
{
    static_assert(sizeof(T) == 0 && "unsupported index type");
};
template <>
struct IndexType<uint16_t>
{
    inline constexpr static VkIndexType type = VK_INDEX_TYPE_UINT16;
};
template <>
struct IndexType<uint32_t>
{
    inline constexpr static VkIndexType type = VK_INDEX_TYPE_UINT32;
};
/**
@note
Reset() //optional
Begin()
    BeginRenderPass()
        BindPipeline()
        Draw()
    EndRenderPass()
End()
*/
class GraphicsCommandBuffer
{
public:
    class SubmitBuilder
    {
    public:
        SubmitBuilder(GraphicsCommandBuffer& commandBuffer) :
            m_CommandBuffer(commandBuffer)
        {
        }
        SubmitBuilder(const SubmitBuilder&) = delete;
        SubmitBuilder& operator=(const SubmitBuilder&) = delete;
        SubmitBuilder(SubmitBuilder&&) = default;
        ~SubmitBuilder()
        {
#ifdef ENABLE_VALIDATION
            if (!m_HasEndSubmit)
            {
                ASSERT(false && "SubmitBuilder has not been ended");
            }
#endif
        }
        SubmitBuilder& Wait(VkSemaphore semaphore, VkPipelineStageFlags stage)
        {
            m_WaitSemaphores.push_back(semaphore);
            m_WaitStages.push_back(stage);
            return *this;
        }
        SubmitBuilder& Signal(VkSemaphore semaphore)
        {
            m_SignalSemaphores.push_back(semaphore);
            return *this;
        }
        SubmitBuilder& Fence(Fence& fence)
        {
            m_Fence = fence.GetHandle();
            return *this;
        }
        VkResult EndSubmit()
        {
#ifdef ENABLE_VALIDATION
            m_HasEndSubmit = true;
#endif
            return m_CommandBuffer.Submit(m_WaitSemaphores.size(), m_WaitSemaphores.data(), m_WaitStages.data(), m_SignalSemaphores.size(), m_SignalSemaphores.data(), m_Fence);
        }

    private:
        std::vector<VkSemaphore> m_WaitSemaphores;
        std::vector<VkPipelineStageFlags> m_WaitStages;
        std::vector<VkSemaphore> m_SignalSemaphores;
        VkFence m_Fence = VK_NULL_HANDLE;
        GraphicsCommandBuffer& m_CommandBuffer;
#ifdef ENABLE_VALIDATION
        bool m_HasEndSubmit = false;
#endif
    };

public:
    static std::optional<GraphicsCommandBuffer> Create(const GraphicsCommandPool& pool);
    static Scope<GraphicsCommandBuffer> CreateScope(const GraphicsCommandPool& pool);
    ~GraphicsCommandBuffer();
    VkCommandBuffer GetHandle() const;
    GraphicsCommandBuffer(const GraphicsCommandBuffer&) = delete;
    GraphicsCommandBuffer& operator=(const GraphicsCommandBuffer&) = delete;
    GraphicsCommandBuffer(GraphicsCommandBuffer&& other) noexcept;
    GraphicsCommandBuffer& operator=(GraphicsCommandBuffer&& other) noexcept;
    void Reset();
    VkResult Begin();
    VkResult BeginSingleTime();

    void SetViewport(float x, float y, float width, float height);
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

    void BindPipeline(const GraphicsPipeline& pipeline);

    VkResult End();
    void Draw(uint32_t vertexCnt);
    void DrawIndexed(uint32_t indexCnt);
    void BeginRenderPass(const RenderPass& renderPass, const FrameBuffer& framebuffer);
    void BeginRenderPass(const RenderPass& renderPass, const FrameBuffer& framebuffer, const Vec4f& clearColor);
    //template <typename... Ts>
    //void BeginRenderPass(const RenderPass& renderPass, const FrameBuffer& framebuffer, Ts... color)
    //{
    //    VkRenderPassBeginInfo renderPassInfo{};
    //    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //    renderPassInfo.renderPass = renderPass.GetHandle();
    //    renderPassInfo.framebuffer = framebuffer.GetHandle();
    //    renderPassInfo.renderArea.offset = {0, 0};
    //    renderPassInfo.renderArea.extent = framebuffer.GetSize();
    //    auto clearColor = make_array(color...);
    //    renderPassInfo.clearValueCount = clearColor.size();
    //    renderPassInfo.pClearValues = clearColor.data();
    //    return vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //}
    void BeginRenderPass(const RenderPass& renderpass,const FrameBuffer& framebuffer,std::span<VkClearValue> clearValues);
    void EndRenderPass();

    SubmitBuilder BeginSubmit();
    VkResult Submit(uint32_t waitCnt, VkSemaphore* waitSemaphores,
                    VkPipelineStageFlags* waitStages, uint32_t signalCnt, VkSemaphore* signalSemaphores, VkFence fence);
    void BindVertexBuffer(Buffer& buffer, uint32_t firstBinding = 0);

    void BindVertexBuffers(const VkBuffer* buffers, uint32_t bufferCnt, uint32_t firstBinding = 0);
    void BindVertexBuffers(const VkBuffer* buffers, uint32_t bufferCnt, uint32_t firstBinding,
                           const VkDeviceSize* offsets);
    void BindVertexBuffers(const std::vector<VkBuffer>& buffers, uint32_t firstBinding = 0);

    void BindIndexBuffer(Buffer& buffer, VkIndexType type, uint32_t offset );
    void BindDescriptorSet(DescriptorSet& descriptorSet, PipelineLayout& pipelineLayout, uint32_t setIndex);
    void UpdatePushConstants(const void* data, uint32_t size, uint32_t offset, PipelineLayout& pipelineLayout, vk::ShaderStage stage);
    void UpdatePushConstantsOnVertexStage(const void* data, uint32_t size, uint32_t offset, PipelineLayout& pipelineLayout);
    void UpdatePushConstantsOnFragmentStage(const void* data, uint32_t size, uint32_t offset, PipelineLayout& pipelineLayout);

private:
private:
    GraphicsCommandBuffer(VkCommandBuffer commandBuffer, const GraphicsCommandPool* commandPool);
    VkCommandBuffer m_CommandBuffer;
    const GraphicsCommandPool* m_CommandPool;
};
}
} // namespace Aether::vk