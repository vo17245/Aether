#pragma once
#include "Backend/Vulkan/GraphicsCommandBuffer.h"
#include <cassert>
#include <variant>
#include "Pipeline.h"
#include "Buffer.h"
#include "CompareOp.h"
#include "RenderPass.h"
#include "Texture2D.h"

namespace Aether::rhi
{

struct TextureUploadRegion
{
    uint32_t x = 0, y = 0;
    uint32_t width = 0, height = 0;
};

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
    void BindPipeline(Pipeline& pipeline);
    /**
     * @brief record a buffer copy command
     */
    void UploadVertexBuffer(StagingBuffer& src, VertexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void UploadIndexBuffer(StagingBuffer& src, IndexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void UploadUniformBuffer(StagingBuffer& src, UniformBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    void UploadUniformBuffer(StagingBuffer& src, RWStructuredBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    // Tightly packed pixels; destination must be in TransferDst layout.
    void UploadTexture(StagingBuffer& src, Texture2D& dst, const TextureUploadRegion& region);
    void SetDepthCompareOp(CompareOp op);
    operator bool() const
    {
        return m_Data.index() != 0;
    }
    vk::GraphicsCommandBuffer& GetVk()
    {
        return std::get<vk::GraphicsCommandBuffer>(m_Data);
    }
    void BeginRenderPass(const RenderPass& pass);
    void EndRenderPass();
    void TextureLayoutTransition(Texture2D& texture, TextureLayout oldLayout, TextureLayout newLayout);
    void CopyVertexBuffer(StagingBuffer& src, VertexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
private:
    std::variant<std::monostate, vk::GraphicsCommandBuffer> m_Data;
};

} // namespace Aether::rhi