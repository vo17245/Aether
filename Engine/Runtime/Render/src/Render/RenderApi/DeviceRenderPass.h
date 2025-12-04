#pragma once
#include "../Vulkan/RenderPass.h"
#include "Render/Vulkan/RenderPass.h"
#include <variant>
#include <cassert>
#include "../Config.h"
#include <Core/Core.h>
namespace Aether
{
enum class DeviceAttachmentLoadOp
{
    Clear,
    Load,
    DontCare,
};
inline constexpr VkAttachmentLoadOp DeviceAttachmentLoadOpToVkLoadOp(DeviceAttachmentLoadOp op)
{
    switch (op)
    {
    case DeviceAttachmentLoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case DeviceAttachmentLoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case DeviceAttachmentLoadOp::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default:
        assert(false && "unknown DeviceAttachmentLoadOp");
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE; // default case
    }
}
enum class DeviceAttachmentStoreOp
{
    DontCare,
    Store,
};
inline constexpr VkAttachmentStoreOp DeviceAttachmentStoreOpToVkStoreOp(DeviceAttachmentStoreOp op)
{
    switch (op)
    {
    case DeviceAttachmentStoreOp::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case DeviceAttachmentStoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    default:
        assert(false && "unknown DeviceAttachmentStoreOp");
        return VK_ATTACHMENT_STORE_OP_DONT_CARE; // default case
    }
}
struct DeviceAttachmentDesc
{
    DeviceAttachmentLoadOp loadOp;
    DeviceAttachmentStoreOp storeOp;
    PixelFormat format;
    bool operator==(const DeviceAttachmentDesc& other) const
    {
        return loadOp == other.loadOp && storeOp == other.storeOp && format == other.format;
    }
};

struct DeviceRenderPassDesc
{
    static const inline constexpr size_t MaxColorAttachments = 8;
    DeviceAttachmentDesc colorAttachments[MaxColorAttachments];
    uint32_t colorAttachmentCount = 0;
    std::optional<DeviceAttachmentDesc> depthAttachment;
    bool operator==(const DeviceRenderPassDesc& other) const
    {
        if (colorAttachmentCount != other.colorAttachmentCount || depthAttachment != other.depthAttachment)
        {
            return false;
        }
        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (colorAttachments[i] != other.colorAttachments[i])
            {
                return false;
            }
        }
        return true;
    }
};
template<>
struct Hash<DeviceRenderPassDesc>
{
    std::size_t operator()(const DeviceRenderPassDesc& value) const
    {
        size_t hash = 0;
        for (size_t i = 0; i < value.colorAttachmentCount; ++i)
        {
            const auto& attachment = value.colorAttachments[i];
            hash ^= std::hash<uint32_t>()(static_cast<uint32_t>(attachment.loadOp));
            hash ^= std::hash<uint32_t>()(static_cast<uint32_t>(attachment.storeOp));
            hash ^= std::hash<PixelFormat>()(attachment.format);
        }
        if (value.depthAttachment)
        {
            const auto& attachment = value.depthAttachment.value();
            hash ^= std::hash<uint32_t>()(static_cast<uint32_t>(attachment.loadOp));
            hash ^= std::hash<uint32_t>()(static_cast<uint32_t>(attachment.storeOp));
            hash ^= std::hash<PixelFormat>()(attachment.format);
        }
        return hash;
    }
};
struct VkRenderPassCreateInfoStorage
{
    VkAttachmentDescription attachs[DeviceRenderPassDesc::MaxColorAttachments + 1] = {};
    VkAttachmentReference colorAttachRefs[DeviceRenderPassDesc::MaxColorAttachments ] = {};
    VkAttachmentReference depthAttachRef;
    VkSubpassDescription subpass = {};
    VkSubpassDependency dependency = {};
    VkRenderPassCreateInfo renderPassInfo = {};
};
void DeviceRenderPassDescToVk(const DeviceRenderPassDesc& desc, VkRenderPassCreateInfoStorage& storage);
class DeviceRenderPass
{
public:
    DeviceRenderPass() = default;
    template <typename T>
    DeviceRenderPass(T&& t) :
        m_Data(std::forward<T>(t))
    {
    }
    DeviceRenderPass(std::monostate) :
        m_Data(std::monostate())
    {
    }
    bool Empty() const
    {
        return m_Data.index() == 0;
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_Data);
    }
    auto& GetData()
    {
        return m_Data;
    }
    const auto& GetData() const
    {
        return m_Data;
    }
    vk::RenderPass& GetVk()
    {
        return std::get<vk::RenderPass>(m_Data);
    }
    const vk::RenderPass& GetVk() const
    {
        return std::get<vk::RenderPass>(m_Data);
    }
    static DeviceRenderPass Create(const DeviceRenderPassDesc& desc)
    {
        assert(DeviceRenderPassDesc::MaxColorAttachments >= desc.colorAttachmentCount && "too many color attachments");
        switch (Render::Api::Vulkan)
        {
        case Render::Api::Vulkan: {
            VkRenderPassCreateInfoStorage info;
            DeviceRenderPassDescToVk(desc, info);
            auto renderPass = vk::RenderPass::Create(info.renderPassInfo);
            if (!renderPass.has_value())
            {
                return DeviceRenderPass(std::monostate());
            }
            return DeviceRenderPass(std::move(renderPass.value()));
        }
        break;
        default:
            assert(false && "unsupported render api");
            return DeviceRenderPass(std::monostate());
        }
    }

private:
    std::variant<std::monostate, vk::RenderPass> m_Data;
};
class DeviceRenderPassView
{
public:
    DeviceRenderPassView() = default;
    template <typename T>
    T& Get()
    {
        return *std::get<T*>(m_Data);
    }
    template <typename T>
    const T& Get() const
    {
        return *std::get<T*>(m_Data);
    }

    DeviceRenderPassView(vk::RenderPass& t) :
        m_Data(&t)
    {
    }
    DeviceRenderPassView(DeviceRenderPass& t)
    {
        if (std::holds_alternative<std::monostate>(t.GetData()))
        {
            m_Data = std::monostate();
        }
        else if (std::holds_alternative<vk::RenderPass>(t.GetData()))
        {
            auto& renderPass = t.Get<vk::RenderPass>();
            m_Data = &renderPass;
        }
        else
        {
            assert(false && "unknown renderpass type");
        }
    }
    DeviceRenderPassView(std::monostate) :
        m_Data(std::monostate())
    {
    }
    DeviceRenderPassView(const DeviceRenderPassView& other) = default;
    DeviceRenderPassView(DeviceRenderPassView&& other) = default;
    DeviceRenderPassView& operator=(const DeviceRenderPassView& other) = default;
    DeviceRenderPassView& operator=(DeviceRenderPassView&& other) = default;
    bool Empty() const
    {
        return m_Data.index() == 0;
    }
    auto& GetData()
    {
        return m_Data;
    }
    const auto& GetData() const
    {
        return m_Data;
    }
    vk::RenderPass& GetVk()
    {
        return *std::get<vk::RenderPass*>(m_Data);
    }
    const vk::RenderPass& GetVk() const
    {
        return *std::get<vk::RenderPass*>(m_Data);
    }

private:
    std::variant<std::monostate, vk::RenderPass*> m_Data; // do not own the object
};

} // namespace Aether