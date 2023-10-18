#pragma once
#include "../Core/Core.h"
#include <string>
#include <vector>
#include <optional>
#include <functional>
AETHER_NAMESPACE_BEGIN
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
		, m_Channel(0)
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
	
private:
	std::unique_ptr<unsigned char,std::function<void(unsigned char*)>> m_Data;
	int m_Width;
	int m_Height;
	int m_Channel;
private:
	//release resource
	static void ReleaseDataCreatedByStb(unsigned char* ptr);
	// use delete[] to release pointer created by new[],if not nullptr
	static void ReleaseDataCreatedByCppNewArray(unsigned char* ptr);
};
AETHER_NAMESPACE_END