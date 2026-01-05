#pragma once
#include "Backend/Vulkan/GraphicsCommandBuffer.h"
#include <cassert>
#include <variant>
#include "Pipeline.h"
#include "Buffer.h"
#include "CompareOp.h"
namespace Aether::rhi
{

class CommandList
{
public:
    CommandList() = default;
    template <typename T>
    CommandList(T&& t) : m_Data(std::forward<T>(t))
    {
    }

    bool Empty() const
    {
        return m_Data.index() == 0;
    }

    void SetViewport(float x, float y, float width, float height);
    void SetScissor(float x, float y, float width, float height);
    void BindPipeline(DevicePipeline& pipeline);
    /**
     * @brief record a buffer copy command
     */
    void UploadVertexBuffer(StagingBuffer& src, VertexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void UploadIndexBuffer(StagingBuffer& src, IndexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void UploadUniformBuffer(StagingBuffer& src, UniformBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void UploadUniformBuffer(StagingBuffer& src, RWStructuredBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void SetDepthCompareOp(CompareOp op);
    operator bool() const
    {
        return m_Data.index() != 0;
    }

private:
    std::variant<std::monostate, vk::GraphicsCommandBuffer> m_Data;
};

} // namespace Aether::rhi