#include "TextureAsset.h"
namespace Aether{
TextureAsset::TextureAsset(Ref<Image> image, const std::string& typeName)
{
	m_Image = image;
	m_TypeName = typeName;
}
}//namespace Aether


