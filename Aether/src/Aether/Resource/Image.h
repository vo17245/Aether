#pragma once
#include "../Core/Core.h"
#include <string>
#include <vector>
#include <optional>
#include <functional>


namespace Aether
{
enum class ImageFormat
{
	RGBA8888,
	RGB888,
	RGBA_FLOAT32,
	RGB_FLOAT32,
	GRAY8,
	GRAY_FLOAT32,
};

//channel == 3  -> RGB888
//channel == 4  -> RGBA8888
class Image
{
public:
	//create a empty image
	Image()
		:m_Data(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_Channel(4)
		, m_Format(ImageFormat::RGBA8888)
	{

	}
	~Image();
	
	Image(Image&& image)noexcept;
	Image(const Image& image);
	inline const int GetWidth()const { return m_Width; }
	inline const int GetHeight()const { return m_Height; }
	inline const int GetChannel()const { return m_Channel; }
	inline const unsigned char* GetData()const { return m_Data.get(); }
	static std::optional<Image> LoadFromMemDataRGB888(const char* data, size_t width, size_t height);
	static std::optional<Image> LoadFromMemDataRGBA8888(const char* data, size_t width, size_t height);
	static std::optional<Image> LoadFromMemDataFormat(unsigned char* data, size_t len, bool flip = true);
	static std::optional<Image> LoadFromFileDataFormat(const std::string& path, bool flip = true);
	static std::optional<Image> 
	LoadFromFileDataFormat2Float32(const std::string& path,bool flip=true);
private:
	std::unique_ptr<unsigned char,std::function<void(unsigned char*)>> m_Data;
	int m_Width;
	int m_Height;
	int m_Channel;
	ImageFormat m_Format;
private:
	//release resource
	static void ReleaseDataCreatedByStb(unsigned char* ptr);
	// use delete[] to release pointer created by new[],if not nullptr
	static void ReleaseDataCreatedByCppNewArray(unsigned char* ptr);
	//release resource created by stb_image stbi_loadf
	static void ReleeaseDataCreatedByStbLoadf(unsigned char* ptr);
};
}//namespace Aether
