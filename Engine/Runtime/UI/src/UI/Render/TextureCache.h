#pragma once
#include <Render/RenderApi.h>
#include <Resource/Resource.h>
#include <Core/Cache.h>
#include "LoadTextureToLinear.h"
#include <Core/Borrow.h>
namespace Aether::UI 
{
    class TextureCache 
    {
    public:
        TextureCache(Borrow<DynamicStagingBuffer> stagingBuffer) :m_StagingBuffer(stagingBuffer)
        {
        }
        Ref<DeviceTexture> Get(const std::string& path)
        {
            auto iter=m_Textures.find(path);
            if(iter!=m_Textures.end())
            {
                return iter->second;
            }
            auto texture=LoadTexture(path);
            if(!texture)
            {
                return nullptr;
            }
            m_Textures[path]=texture;
            return texture;
        }
        void ClearNotUsedTextures()
        {
            for(auto iter=m_Textures.begin();iter!=m_Textures.end();)
            {
                if(iter->second.use_count()==1) // only used by this cache
                {
                    iter=m_Textures.erase(iter);
                }
                else 
                {
                    ++iter;
                }
            }
        }
    private:
        Ref<DeviceTexture> LoadTexture(const std::string& path)
        {
            auto opt=m_Finder.Find(path);
            if(!opt)
            {
                return nullptr;
            }
            auto& assetInfo = *opt;
            if(assetInfo.info->type != Resource::AssetType::Image)
            {
                return nullptr;
            }
            auto& imageInfo=*(Resource::ImageInfo*)assetInfo.info.get();
            if(imageInfo.colorSpace==Resource::ColorSpace::SRGB)
            {
                auto texture=LoadTextureToLinear(path, imageInfo, *m_StagingBuffer);
                if(!texture)
                {
                    return nullptr;
                }
                auto textureRef = CreateRef<DeviceTexture>(std::move(*texture));
                
                return textureRef;
            }
            else 
            {
                assert(false&& "Only sRGB color space is supported for now");
                return nullptr;
            }

        }
        std::unordered_map<std::string, Ref<DeviceTexture>> m_Textures;
        Resource::Finder m_Finder;        
        Borrow<DynamicStagingBuffer> m_StagingBuffer; // for loading textures
    };
}