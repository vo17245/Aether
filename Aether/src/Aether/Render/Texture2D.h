#pragma once
#include "../Core/Core.h"
#include <string>
#include "../Resource/Image.h"
#include "OpenGLApi.h"
namespace Aether
{

class Texture2D
{

public:
	enum class Format:uint32_t
	{
		RGB=0,
		RGBA=1,
	};
	enum class ScalarType:uint32_t
	{
		UBYTE=0,
		FLOAT32=1,
	};
	struct Spec
	{
		size_t width;
		size_t height;
		Format format;
		Format internalFormat;
		ScalarType dataType;
	};
public:
	Texture2D(const Image& image);
	Texture2D(size_t width, size_t height);
	Texture2D(const Texture2D&) = delete;
	Texture2D(Texture2D&&) = delete;
	Texture2D(const Spec& spec);
	~Texture2D();
	void Bind();
	void Unbind();
	void Unbind(uint32_t slot);
	void Bind(uint32_t slot);
	inline size_t GetWidth() { return m_Width; }
	inline size_t GetHeight() { return m_Height; }
	inline RendererId GetRendererId() { return m_RendererId; }
	static Ref<Texture2D> Create(const Image& image)
	{
		return CreateRef<Texture2D>(image);
	}
private:
	RendererId m_RendererId;
	size_t m_Width;
	size_t m_Height;
};
}//namespace Aether
