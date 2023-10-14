#pragma once

#include "Image.h"
#include <string>
#include <memory>
AETHER_NAMESPACE_BEGIN
class TextureAsset
{
public:
	TextureAsset(const std::string& path, const std::string& typeName);
	~TextureAsset(){}
	inline const std::string& GetPath() { return m_Path; }
	inline const std::string& GetTypeName() { return m_TypeName; }
	inline const Image& GetImage() { return *m_Image; }
	inline const std::shared_ptr<Image> GetImageRef() { return m_Image; }
private:
	std::string m_Path;
	std::string m_TypeName;
	std::shared_ptr<Image> m_Image;
    
};
AETHER_NAMESPACE_END