#pragma once
#include "../Core/Core.h"
#include <string>
AETHER_NAMESPACE_BEGIN
// 有三个或者四个通道 rgb / rgba 每个通道8bit
class Image
{
public:
	Image(const std::string& path,bool flip=true);
	Image(const unsigned char* data,size_t size,bool flip=true);
	~Image();
	inline const int GetWidth()const { return m_Width; }
	inline const int GetHeight()const { return m_Height; }
	inline const int GetChannel()const { return m_Channel; }
	inline const unsigned char* GetData()const { return m_Data; }
private:
	unsigned char* m_Data;
	int m_Width;
	int m_Height;
	int m_Channel;
};
AETHER_NAMESPACE_END