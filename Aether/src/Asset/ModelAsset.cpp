#include "ModelAsset.h"
#include <iostream>
#include "../Core/Log.h"


AETHER_NAMESPACE_BEGIN
std::optional<Ref<TextureAsset>> ModelAsset::LoadEmbeddedTexture(size_t index, aiTexture** textures, const std::string& typeName)
{
    auto iter = m_EmbeddedTextureCache.find(index);
    if (iter != m_EmbeddedTextureCache.end())
    {
        return iter->second;
    }
    aiTexture* texture = textures[index];
    if (texture->mHeight == 0)
    {

        auto compressedImageLoadres = Image::LoadFromMemDataFormat((unsigned char*)texture->pcData, texture->mWidth);
        if (!compressedImageLoadres)
        {
            AETHER_DEBUG_LOG_ERROR("Failed to load embedded compressed image {0}:{1}", __FILE__, __LINE__);
            return std::nullopt;
        }
        auto image = CreateRef<Image>(std::move(compressedImageLoadres.value()));
        auto textureAsset = CreateRef<TextureAsset>(image, typeName);
        return textureAsset;

    }
    else
    {
        auto loadRes = Image::LoadFromMemDataRGBA8888(
            reinterpret_cast<const char*>(texture->pcData),
            texture->mWidth,
            texture->mHeight);
        if (!loadRes)
        {
            AETHER_DEBUG_LOG_ERROR("Failed to load embedded RGBA8888 image");
            return std::nullopt;
        }
        auto image = CreateRef<Image>(std::move(loadRes.value()));
        auto textureAsset = CreateRef<TextureAsset>(image, typeName);
        return textureAsset;
    }
}

std::optional<Ref<TextureAsset>> ModelAsset::LoadFileTexture(const std::string& path, const std::string& typeName)
{
    auto loadRes = Image::LoadFromFileDataFormat(path);
    if (!loadRes)return std::nullopt;
    auto image=CreateRef<Image>(std::move(loadRes.value()));
    auto texture = CreateRef<TextureAsset>(image,typeName);
    return std::optional<Ref<TextureAsset>>(texture);
}
AETHER_NAMESPACE_END