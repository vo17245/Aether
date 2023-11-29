#include "FrameBuffer.h"
#include "../Core/Log.h"
namespace Aether
{
	static RendererId CreateFrameBuffer(Ref<Texture2D>& tex, RendererId& rb)
	{
		
		unsigned int framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		

		// 将它附加到当前绑定的帧缓冲对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_2D, 
			tex->GetRendererId(), 0);
		
		glGenRenderbuffers(1, &rb);
		glBindRenderbuffer(GL_RENDERBUFFER, rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex->GetWidth(),
			tex->GetHeight());
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			AETHER_DEBUG_LOG_ERROR("framebuffer is not complete");
		}
		return framebuffer;
	}
	FrameBuffer::FrameBuffer(Ref<Texture2D>& tex)
		:m_RendererId(0),m_Texture(tex)
	{
		m_RendererId = CreateFrameBuffer(tex, m_RenderBuferId);
	}
	FrameBuffer::FrameBuffer(size_t width, size_t height)
	{
		m_Texture=CreateRef<Texture2D>(width, height);
		m_RendererId = CreateFrameBuffer(m_Texture,m_RenderBuferId);
	}
	FrameBuffer::~FrameBuffer()
	{
		
		glDeleteFramebuffers(1, &m_RendererId);
		glDeleteRenderbuffers(1, &m_RenderBuferId);
		
	}
	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
	}
	void FrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}