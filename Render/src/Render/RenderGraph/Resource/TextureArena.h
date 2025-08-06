#pragma once
#include <Render/RenderApi.h>
namespace Aether::RenderGraph
{
    class TextureArena
    {
    public:
        void AddDependency(DeviceImageView* imageView,DeviceTexture* texture)
        {
            if(m_TextureToImageViewMap.find(texture) == m_TextureToImageViewMap.end())
            {
                m_TextureToImageViewMap[texture] = std::vector<DeviceImageView*>();
            }
            m_TextureToImageViewMap[texture].push_back(imageView);
        }
        void AddDependency(DeviceFrameBuffer* frameBuffer,DeviceImageView* imageView)
        {
            if(m_ImageViewToFrameBufferMap.find(imageView) == m_ImageViewToFrameBufferMap.end())
            {
                m_ImageViewToFrameBufferMap[imageView] = std::vector<DeviceFrameBuffer*>();
            }
            m_ImageViewToFrameBufferMap[imageView].push_back(frameBuffer);
        }
        void DestroyImageView(DeviceImageView* imageView)
        {
            // destroy frame buffers that depend on this image view
            auto iter = m_ImageViewToFrameBufferMap.find(imageView);
            if (iter != m_ImageViewToFrameBufferMap.end())
            {
                for (auto frameBuffer : iter->second)
                {
                    auto fbIter = m_FrameBufferMap.find(frameBuffer);
                    if (fbIter != m_FrameBufferMap.end())
                    {
                        m_FrameBuffers.erase(fbIter->second);
                        m_FrameBufferMap.erase(fbIter);
                    }
                }
                m_ImageViewToFrameBufferMap.erase(iter);
            }
            // destroy image view
            auto imageViewIter = m_ImageViewMap.find(imageView);
            if (imageViewIter != m_ImageViewMap.end())
            {
                m_ImageViews.erase(imageViewIter->second);
                m_ImageViewMap.erase(imageViewIter);
            }

        }
        void DestroyTexture(DeviceTexture* texture)
        {
            // destroy image views that depend on this texture
            auto iter = m_TextureToImageViewMap.find(texture);
            if (iter != m_TextureToImageViewMap.end())
            {
                for (auto imageView : iter->second)
                {
                    DestroyImageView(imageView);
                }
                m_TextureToImageViewMap.erase(iter);
            }
            // destroy texture
            auto textureIter = m_TextureMap.find(texture);
            if (textureIter != m_TextureMap.end())
            {
                m_Textures.erase(textureIter->second);
                m_TextureMap.erase(textureIter);
            }
        }
        void DestroyFrameBuffer(DeviceFrameBuffer* frameBuffer)
        {
            // destroy frame buffer
            auto iter = m_FrameBufferMap.find(frameBuffer);
            if (iter != m_FrameBufferMap.end())
            {
                m_FrameBuffers.erase(iter->second);
                m_FrameBufferMap.erase(iter);
            }
        }
        void AddFrameBuffer(Scope<DeviceFrameBuffer>&& frameBuffer)
        {
            m_FrameBuffers.push_back(std::move(frameBuffer));
            auto iter = m_FrameBuffers.end();
            --iter;
            m_FrameBufferMap[iter->get()] = iter;
        }
        void AddImageView(Scope<DeviceImageView>&& imageView)
        {
            m_ImageViews.push_back(std::move(imageView));
            auto iter = m_ImageViews.end();
            --iter;
            m_ImageViewMap[iter->get()] = iter;
        }
        void AddTexture(Scope<DeviceTexture>&& texture)
        {
            m_Textures.push_back(std::move(texture));
            auto iter = m_Textures.end();
            --iter;
            m_TextureMap[iter->get()] = iter;
        }
        bool IsFrameBufferValid(DeviceFrameBuffer* frameBuffer) const
        {
            return m_FrameBufferMap.find(frameBuffer) != m_FrameBufferMap.end();
        }
        bool IsImageViewValid(DeviceImageView* imageView) const
        {
            return m_ImageViewMap.find(imageView) != m_ImageViewMap.end();
        }
        bool IsTextureValid(DeviceTexture* texture) const
        {
            return m_TextureMap.find(texture) != m_TextureMap.end();
        }
    private:
        std::list<Scope<DeviceFrameBuffer>> m_FrameBuffers;
        std::unordered_map<DeviceFrameBuffer*, typename std::list<Scope<DeviceFrameBuffer>>::iterator> m_FrameBufferMap;
        std::list<Scope<DeviceImageView>> m_ImageViews;
        std::unordered_map<DeviceImageView*,typename std::list<Scope<DeviceImageView>>::iterator> m_ImageViewMap;
        
        std::unordered_map<DeviceImageView*, std::vector<DeviceFrameBuffer*>> m_ImageViewToFrameBufferMap;
        std::list<Scope<DeviceTexture>> m_Textures;
        std::unordered_map<DeviceTexture*, typename std::list<Scope<DeviceTexture>>::iterator> m_TextureMap;
        std::unordered_map<DeviceTexture*, std::vector<DeviceImageView*>> m_TextureToImageViewMap;
    };
}