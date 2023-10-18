#include "Texture2D.h"
#include "OpenGLApi.h"
AETHER_NAMESPACE_BEGIN
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

Texture2D::Texture2D(const Image& image,uint32_t slot)
	:m_Slot(slot)
{
	OpenGLApi::ActivateTexture(m_Slot);
	m_RendererId = CreateTexture(image);
}

Texture2D::~Texture2D()
{
	GLCall(glDeleteTextures(1, &m_RendererId));
}

void Texture2D::Bind()
{
	OpenGLApi::ActivateTexture(m_Slot);
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererId));
}

void Texture2D::Unbind()
{
	OpenGLApi::ActivateTexture(m_Slot);
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture2D::SetSlot(uint32_t slot)
{
	m_Slot = slot;
}
AETHER_NAMESPACE_END