#include <Render/RHI/CommandList.h>
#include <Render/RHI/Backend/Vulkan/Transfer.h>
namespace Aether::rhi
{
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
    }
    void CommandList::UploadIndexBuffer(StagingBuffer& src, IndexBuffer& dst, size_t size, size_t srcOffset, size_t dstOffset)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan:
            vk::AsyncCopyBuffer(std::get<vk::GraphicsCommandBuffer>(m_Data), src.GetVk(), dst.GetVk(), size, srcOffset, dstOffset);
            break;
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
            info.clearValue.color.float32[0] = attachment.clearColor.x();
            info.clearValue.color.float32[1] = attachment.clearColor.y();
            info.clearValue.color.float32[2] = attachment.clearColor.z();
            info.clearValue.color.float32[3] = attachment.clearColor.w();
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

