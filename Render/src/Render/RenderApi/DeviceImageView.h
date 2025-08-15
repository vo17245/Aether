#pragma once
#include "../Vulkan/Texture2D.h"
#include "Render/Config.h"
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"
#include <cassert>
#include <variant>
#include <expected>
#include "../Config.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "DeviceBuffer.h"
#include "Render/Vulkan/ImageView.h"
#include "vulkan/vulkan_core.h"
#include <Core/Core.h>
namespace Aether
{
struct DeviceImageViewDesc{
    bool operator==(const DeviceImageViewDesc& other) const
    {
        return true; // Placeholder, as DeviceImageViewDesc is empty
    }
};
template<>
struct Hash<DeviceImageViewDesc>
{
    std::size_t operator()(const DeviceImageViewDesc&) const
    {
        return 0; // Placeholder hash function, as DeviceImageViewDesc is empty
    }
};
class DeviceImageView
{
public:
    bool Empty() const
    {
        return m_ImageView.index() == 0;
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_ImageView);
    }
    template <typename T>
    const T& Get() const
    {
        return std::get<T>(m_ImageView);
    }
    DeviceImageView(std::monostate) :
        m_ImageView(std::monostate{})
    {
    }
    DeviceImageView(vk::ImageView&& imageView) :
        m_ImageView(std::move(imageView))
    {
    }
    DeviceImageView()
    {
    }
    DeviceImageView(DeviceImageView&&) = default;
    DeviceImageView& operator=(DeviceImageView&&) = default;
    vk::ImageView& GetVk()
    {
        return std::get<vk::ImageView>(m_ImageView);
    }
    const vk::ImageView& GetVk() const
    {
        return std::get<vk::ImageView>(m_ImageView);
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::ImageView> m_ImageView;
};
} // namespace Aether