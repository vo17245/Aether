#include "Image.h"
#include "stb/stb_image.h"
#include <assert.h>
AETHER_NAMESPACE_BEGIN
Image::Image(const std::string& path,bool flip)
    :m_Width(0), m_Height(0), m_Channel(0)
{
    stbi_set_flip_vertically_on_load(flip);
    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channel, 0);
    assert(data != nullptr&&"Failed to load image");
    m_Data = data;
}

Image::Image(const unsigned char* data, size_t size,bool flip)
    :m_Width(0),m_Height(0),m_Channel(0)
{
    stbi_set_flip_vertically_on_load(flip);
    m_Data = stbi_load_from_memory(data,
        static_cast<int>(size), &m_Width, &m_Height, &m_Channel, 0);
}


Image::~Image()
{

    STBI_FREE(m_Data);
}
AETHER_NAMESPACE_END