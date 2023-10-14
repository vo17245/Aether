#include "TextureAsset.h"
AETHER_NAMESPACE_BEGIN
TextureAsset::TextureAsset(const std::string& path, const std::string& typeName)
	:m_Path(path),m_TypeName(typeName)
{
	m_Image = std::make_shared<Image>(path);
}
AETHER_NAMESPACE_END