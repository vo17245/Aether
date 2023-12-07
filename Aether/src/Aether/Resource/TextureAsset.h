#pragma once

#include "Image.h"
#include <string>
#include <memory>
AETHER_NAMESPACE_BEGIN
class TextureAsset
{
public:
	TextureAsset(Ref<Image> image, const std::string& typeName);
	~TextureAsset(){}
	inline const std::string& GetTypeName() { return m_TypeName; }
	inline const Image& GetImage() { return *m_Image; }
	inline const std::shared_ptr<Image> GetImageRef() { return m_Image; }
private:
	std::string m_TypeName;
	Ref<Image> m_Image;
    
};
AETHER_NAMESPACE_END