#include "Texture2D.h"

namespace Aether
{

static uint32_t CreateTexture(const Image& image)
{
	uint32_t rendererId;
	GLCall(glGenTextures(1, &rendererId));
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererId));
	if (image.GetChannel() == 3)
	{
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			image.GetWidth(), image.GetHeight(),
			0, GL_RGB, GL_UNSIGNED_BYTE,
			image.GetData()));
	}
	else if (image.GetChannel() == 4)
	{
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			image.GetWidth(), image.GetHeight(),
			0, GL_RGBA, GL_UNSIGNED_BYTE,
			image.GetData()));
	}

	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	return rendererId;
}
static RendererId CreateTexture(size_t width, size_t height)
{
	unsigned int texture;
	GLCall(glGenTextures(1, &texture));
	GLCall(glBindTexture(GL_TEXTURE_2D, texture));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	return texture;
}
Texture2D::Texture2D(const Image& image)
	:m_RendererId(0),m_Width(image.GetWidth()),m_Height(image.GetHeight())
{
	m_RendererId = CreateTexture(image);
}

Texture2D::Texture2D(size_t width, size_t height)
	:m_RendererId(0),m_Width(width),m_Height(height)
{
	m_RendererId = CreateTexture(width, height);
}

Texture2D::~Texture2D()
{
	GLCall(glDeleteTextures(1, &m_RendererId));
}

void Texture2D::Bind()
{
	
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererId));
}

void Texture2D::Unbind()
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture2D::Unbind(uint32_t slot)
{
	OpenGLApi::ActivateTexture(slot);
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture2D::Bind(uint32_t slot)
{
	OpenGLApi::ActivateTexture(slot);
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererId));
}

}//namespace Aether
