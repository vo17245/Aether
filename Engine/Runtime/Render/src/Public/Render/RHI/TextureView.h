#pragma once
#include "Backend/Vulkan/Texture2D.h"
#include "Render/Config.h"
#include "Render/PixelFormat.h"
#include <cassert>
#include <variant>
#include <expected>
#include "../Config.h"
#include "Backend/Vulkan/Buffer.h"
#include "Backend/Vulkan/GlobalRenderContext.h"
#include "Buffer.h"
#include "Backend/Vulkan/ImageView.h"
#include <Core/Core.h>
namespace Aether::rhi
{
struct TextureViewDesc
{
    bool operator==(const TextureViewDesc& other) const
    {
        return true; // Placeholder, as ImageViewDesc is empty
    }
};
} // namespace Aether::rhi
namespace Aether
{
template <>
struct Hash<rhi::TextureViewDesc>
{
    std::size_t operator()(const rhi::TextureViewDesc&) const
    {
        return 0; // Placeholder hash function, as ImageViewDesc is empty
    }
};
} // namespace Aether
namespace Aether::rhi
{

class ImageView
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
    ImageView(std::monostate) : m_ImageView(std::monostate{})
    {
    }
    ImageView(vk::ImageView&& imageView) : m_ImageView(std::move(imageView))
    {
    }
    ImageView()
    {
    }
    ImageView(ImageView&&) = default;
    ImageView& operator=(ImageView&&) = default;
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
} // namespace Aether::rhi