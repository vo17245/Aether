#include "Texture2D.h"
#include "Aether/Core/Assert.h"
namespace Aether
{
static uint32_t CreateTextureFloat(const Image& image)
{
	GLenum gl_image_format=GL_RGB;
	if(image.GetChannel()==3)
	{
		gl_image_format=GL_RGB;
	}
	else if(image.GetChannel()==4)
	{
		gl_image_format=GL_RGBA;
	}
	else
	{
		AETHER_ASSERT(false&&"Unsupported image format");
	}
	unsigned int hdrTexture;
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, image.GetWidth(), image.GetHeight(), 0, gl_image_format, GL_FLOAT, image.GetData()); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return hdrTexture;

}
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

static RendererId CreateTextureEx(const Image& image)
{
	ImageFormat format=image.GetFormat();
	
	switch (format)
	{
	case ImageFormat::RGBA8888:

		return CreateTexture(image);
		break;
	case ImageFormat::RGB888:
		return CreateTexture(image);
		break;
	case ImageFormat::RGBA_FLOAT32:
	{
		RendererId texture;
		GLCall(glGenTextures(1, &texture));
		GLCall(glBindTexture(GL_TEXTURE_2D, texture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, image.GetWidth(),
			image.GetHeight(), 0, GL_RGBA, GL_FLOAT, image.GetData()));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
		break;
	case ImageFormat::RGB_FLOAT32:
	{
		RendererId texture;
		GLCall(glGenTextures(1, &texture));
		GLCall(glBindTexture(GL_TEXTURE_2D, texture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, image.GetWidth(),
			image.GetHeight(), 0, GL_RGB, GL_FLOAT, image.GetData()));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		return texture;
	}
		break;
	case ImageFormat::GRAY8:
		AETHER_ASSERT(false && "unsupport format");
		break;
	case ImageFormat::GRAY_FLOAT32:
		AETHER_ASSERT(false && "unsupport format");
		break;
	default:
		AETHER_ASSERT(false && "unsupport format");
		break;
	}
}
Texture2D::Texture2D(const Image& image)
	:m_RendererId(0),m_Width(image.GetWidth()),m_Height(image.GetHeight())
{
	m_RendererId = CreateTextureEx(image);
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
