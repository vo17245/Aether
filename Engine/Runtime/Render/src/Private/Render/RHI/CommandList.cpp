#include <Render/RHI/CommandList.h>
#include <Render/RHI/Backend/Vulkan/Transfer.h>
#include <limits>
#include <stdexcept>
namespace Aether::rhi
{
void CommandList::DownloadTexture(Texture2D& src, StagingBuffer& dst,
                                  std::span<const TextureDownloadRegion> regions)
{
    if (regions.empty()) return;
    if (src.GetFormat() != PixelFormat::R_UINT32 && src.GetFormat() != PixelFormat::R_FLOAT32)
        throw std::invalid_argument("texture download supports only R32_UINT and R32_SFLOAT");
    if (!(src.GetVk().GetVkUsageFlags() & VK_IMAGE_USAGE_TRANSFER_SRC_BIT))
        throw std::invalid_argument("texture download source lacks TRANSFER_SRC usage");
    const auto usage = static_cast<std::underlying_type_t<vk::Buffer::Usage>>(dst.GetVk().GetUsage());
    if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT))
        throw std::invalid_argument("texture download destination lacks TRANSFER_DST usage");
    std::vector<VkBufferImageCopy> copies;
    copies.reserve(regions.size());
    const size_t texelBytes = PixelFormatSize(src.GetFormat());
    for (const auto& region : regions)
    {
        if (!region.width || !region.height || region.x > src.GetWidth()
            || region.width > src.GetWidth() - region.x || region.y > src.GetHeight()
            || region.height > src.GetHeight() - region.y || region.bufferOffset % 4 != 0)
            throw std::invalid_argument("texture download region is invalid");
        if (region.width > std::numeric_limits<size_t>::max() / region.height
            || size_t(region.width) * region.height > std::numeric_limits<size_t>::max() / texelBytes)
            throw std::invalid_argument("texture download region size overflows");
        const size_t bytes = size_t(region.width) * region.height * texelBytes;
        if (region.bufferOffset > dst.GetSize() || bytes > dst.GetSize() - region.bufferOffset)
            throw std::invalid_argument("texture download region exceeds destination buffer");
        VkBufferImageCopy copy{};
        copy.bufferOffset = region.bufferOffset;
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;
        copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copy.imageOffset = {static_cast<int32_t>(region.x), static_cast<int32_t>(region.y), 0};
        copy.imageExtent = {region.width, region.height, 1};
        copies.push_back(copy);
    }
    auto& commandBuffer = GetVk();
    vkCmdCopyImageToBuffer(commandBuffer.GetHandle(), src.GetVk().GetHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.GetVk().GetHandle(),
                           static_cast<uint32_t>(copies.size()), copies.data());
    VkBufferMemoryBarrier barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = dst.GetVk().GetHandle();
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(commandBuffer.GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                         0, 0, nullptr, 1, &barrier, 0, nullptr);
}

void CommandList::UploadTexture(StagingBuffer& src, Texture2D& dst, const TextureUploadRegion& region)
{
    assert(region.width > 0 && region.height > 0);
    assert(region.x <= dst.GetWidth() && region.width <= dst.GetWidth() - region.x);
    assert(region.y <= dst.GetHeight() && region.height <= dst.GetHeight() - region.y);
    assert(src.GetSize() >= static_cast<size_t>(region.width) * region.height * PixelFormatSize(dst.GetFormat()));
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageOffset = {static_cast<int32_t>(region.x), static_cast<int32_t>(region.y), 0};
    copy.imageExtent = {region.width, region.height, 1};
    vkCmdCopyBufferToImage(GetVk().GetHandle(), src.GetVk().GetHandle(), dst.GetVk().GetHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

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
    void CommandList::BindPipeline(Pipeline& pipeline)
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
    void CommandList::CopyVertexBuffer(StagingBuffer& src, VertexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            vk::AsyncCopyBuffer(std::get<vk::GraphicsCommandBuffer>(m_Data), src.GetVk(), dst.GetVk(), size, srcOffset, dstOffset);
        }
        break;
        default:
            assert(false && "unsupported api");
            break;
        }
    }
    void CommandList::UploadVertexBuffer(StagingBuffer& src, VertexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset)
    {
        CopyVertexBuffer(src, dst, size, srcOffset, dstOffset);
        if (Render::Config::RenderApi == Render::Api::Vulkan)
        {
            auto& commandBuffer = std::get<vk::GraphicsCommandBuffer>(m_Data);
            VkBufferMemoryBarrier barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = dst.GetVk().GetHandle();
            barrier.offset = dstOffset;
            barrier.size = size;
            vkCmdPipelineBarrier(commandBuffer.GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        }
    }
    void CommandList::UploadIndexBuffer(StagingBuffer& src, IndexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& commandBuffer = std::get<vk::GraphicsCommandBuffer>(m_Data);
            vk::AsyncCopyBuffer(commandBuffer, src.GetVk(), dst.GetVk(), size, srcOffset, dstOffset);
            VkBufferMemoryBarrier barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = dst.GetVk().GetHandle();
            barrier.offset = dstOffset;
            barrier.size = size;
            vkCmdPipelineBarrier(commandBuffer.GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
            break;
        }
        default:
            assert(false && "unsupported api");
            break;
        }
    }
    void CommandList::TextureLayoutTransition(Texture2D& texture, TextureLayout oldLayout, TextureLayout newLayout)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan:
            texture.GetVk().AsyncTransitionLayout(std::get<vk::GraphicsCommandBuffer>(m_Data), RHITextureLayoutToVk(oldLayout), RHITextureLayoutToVk(newLayout));
            break;
        default:
            assert(false && "unsupported api");
            break;
        }
    }
    namespace
    {
    VkAttachmentLoadOp ToVkAttachmentLoadOp(AttachmentLoadOp op)
    {
        switch (op)
        {
        case AttachmentLoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoadOp::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            assert(false && "unknown attachment load operation");
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    VkAttachmentStoreOp ToVkAttachmentStoreOp(AttachmentStoreOp op)
    {
        switch (op)
        {
        case AttachmentStoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case AttachmentStoreOp::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:
            assert(false && "unknown attachment store operation");
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    }
    } // namespace

    void CommandList::BeginRenderPass(const RenderPass& pass)
    {
        if (!std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
        {
            assert(false && "unsupported command buffer type");
            return;
        }
        if (pass.colorAttachments.empty() && !pass.depthAttachment)
        {
            assert(false && "render pass must contain an attachment");
            return;
        }

        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        colorAttachments.reserve(pass.colorAttachments.size());
        for (const auto& attachment : pass.colorAttachments)
        {
            auto& view = attachment.view->GetVk();
            if (width == 0 || height == 0)
            {
                width = view.GetWidth();
                height = view.GetHeight();
            }
            VkRenderingAttachmentInfo info{};
            info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            info.imageView = view.GetHandle();
            info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            info.loadOp = ToVkAttachmentLoadOp(attachment.loadOp);
            info.storeOp = ToVkAttachmentStoreOp(attachment.storeOp);
            if (attachment.clearColorIsUint)
                for (std::size_t channel = 0; channel < 4; ++channel)
                    info.clearValue.color.uint32[channel] = attachment.clearColorUint[channel];
            else
            {
                info.clearValue.color.float32[0] = attachment.clearColor.x();
                info.clearValue.color.float32[1] = attachment.clearColor.y();
                info.clearValue.color.float32[2] = attachment.clearColor.z();
                info.clearValue.color.float32[3] = attachment.clearColor.w();
            }
            colorAttachments.push_back(info);
        }

        VkRenderingAttachmentInfo depthAttachment{};
        if (pass.depthAttachment)
        {
            const auto& attachment = *pass.depthAttachment;
            auto& view = attachment.view->GetVk();
            if (width == 0 || height == 0)
            {
                width = view.GetWidth();
                height = view.GetHeight();
            }
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView = view.GetHandle();
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = ToVkAttachmentLoadOp(attachment.loadOp);
            depthAttachment.storeOp = ToVkAttachmentStoreOp(attachment.storeOp);
            depthAttachment.clearValue.depthStencil = {attachment.clearDepth, attachment.clearStencil};
        }

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.extent = {width, height};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
        renderingInfo.pColorAttachments = colorAttachments.data();
        renderingInfo.pDepthAttachment = pass.depthAttachment ? &depthAttachment : nullptr;
        vkCmdBeginRendering(std::get<vk::GraphicsCommandBuffer>(m_Data).GetHandle(), &renderingInfo);
    }

    void CommandList::EndRenderPass()
    {
        if (!std::holds_alternative<vk::GraphicsCommandBuffer>(m_Data))
        {
            assert(false && "unsupported command buffer type");
            return;
        }
        vkCmdEndRendering(std::get<vk::GraphicsCommandBuffer>(m_Data).GetHandle());
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

