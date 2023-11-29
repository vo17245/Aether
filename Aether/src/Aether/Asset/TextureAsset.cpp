#include "TextureAsset.h"
AETHER_NAMESPACE_BEGIN
TextureAsset::TextureAsset(Ref<Image> image, const std::string& typeName)
{
	m_Image = image;
	m_TypeName = typeName;
}
AETHER_NAMESPACE_END


