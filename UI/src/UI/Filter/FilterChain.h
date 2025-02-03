#pragma once
#include <vector>
#include <functional>
#include "Render/PixelFormat.h"
#include "Render/RenderApi.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/RenderApi/DeviceTexture.h"
namespace Aether::UI
{
class FilterChain
{
public:
    /**
     * @note 为了避免在std::function(lambda)中保存数据,
     * 使用时，FilterI只捕获引用，也就是说，filter使用的资源的所有权不由filter所有
     */
    using FilterI = std::function<
        std::optional<std::string>(const DeviceTexture& from,
                                   DeviceFrameBufferView to,
                                   DeviceCommandBufferView commandBuffer)>;
    /**
     * @param renderPass 用于创建临时texture的renderPass
     * @note 在vulkan后端,传入的renderPass需要和Render时传入的frameBuffer兼容
    */
    FilterChain(DeviceRenderPassView renderPass) :
        m_RenderPass(renderPass)
    {
    }
    void Push(const FilterI& filter)
    {
        m_Filters.push_back(filter);
    }
    std::optional<std::string> Render(const DeviceTexture& from, DeviceFrameBufferView to, DeviceCommandBufferView commandBuffer)
    {
        if (m_Filters.empty())
        {
            return std::nullopt;
        }
        if (m_Filters.size() == 1)
        {
            return m_Filters[0](from, to, commandBuffer);
        }
        // 2 or more filters
        CreateTempTextureIfNesscessary(from.GetWidth(), from.GetHeight());
        // first filter
        auto err = m_Filters[0](from, m_FrameBuffers[0], commandBuffer);
        if (err)
        {
            return err;
        }
        
        // middle filters
        DeviceTexture* at=&m_Textures[0];
        DeviceFrameBuffer* af=&m_FrameBuffers[1];
        DeviceTexture* bt=&m_Textures[1];
        DeviceFrameBuffer* bf=&m_FrameBuffers[0];
        for (size_t i = 1; i < m_Filters.size() - 1; i++)
        {
            err = m_Filters[i](*at, *af, commandBuffer);
            if (err)
            {
                return err;
            }
            std::swap(at,bt);
            std::swap(af,bf);
        }
        // last filter
        err = m_Filters.back()(*at, to, commandBuffer);
        if (err)
        {
            return err;
        }
        return std::nullopt;
    }

private:
    void CreateTempTextureIfNesscessary(size_t width,size_t height)
    {
        if(m_Textures[0].Empty())
        {
            CreateTempTexture(width,height);
            return;
        }
        if(m_Textures[0].GetWidth()<width || m_Textures[0].GetHeight()<height)
        {
            CreateTempTexture(width,height);
        }
    }
    void CreateTempTexture(size_t width, size_t height)
    {
        m_Textures[0] = DeviceTexture::CreateForTexture(width, height, PixelFormat::RGBA8888);
        m_Textures[1] = DeviceTexture::CreateForTexture(width, height, PixelFormat::RGBA8888);
        m_FrameBuffers[0] = DeviceFrameBuffer::CreateFromTexture(m_RenderPass, m_Textures[0]);
        m_FrameBuffers[1] = DeviceFrameBuffer::CreateFromTexture(m_RenderPass, m_Textures[1]);
    }
    std::vector<FilterI> m_Filters;
    DeviceTexture m_Textures[2];
    DeviceFrameBuffer m_FrameBuffers[2];
    DeviceRenderPassView m_RenderPass;
};
} // namespace Aether::UI