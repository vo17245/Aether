#include "FrameBuffer.h"
#include "../Core/Log.h"
namespace Aether
{
	static RendererId CreateFrameBuffer(Ref<Texture2D>& tex)
	{
		//RendererId fb;
		//glGenFramebuffers(1, &fb);
		//glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//glTexImage2D(
		//	GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
		//	tex->GetWidth(),
		//	tex->GetHeight(), 0,
		//	GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
		//);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex-//>GetRendererId(), 0);
		//unsigned int rbo;
		//glGenRenderbuffers(1, &rbo);
		//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex->GetWidth(),
		//	tex->GetHeight());
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, //GL_RENDERBUFFER, rbo);
		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		//{
		//	debug_log("framebuffer is complete");
		//}
		//else
		//{
		//	debug_log_error("framebuffer is not complete");
		//}
		//return fb;
		unsigned int framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		

		// 将它附加到当前绑定的帧缓冲对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_2D, 
			tex->GetRendererId(), 0);
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex->GetWidth(),
			tex->GetHeight());
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			AETHER_DEBUG_LOG_ERROR("framebuffer is not complete");
		}
		return framebuffer;
	}
	FrameBuffer::FrameBuffer(Ref<Texture2D>& tex)
		:m_RendererId(0),m_Texture(tex)
	{
		m_RendererId = CreateFrameBuffer(tex);
	}
	FrameBuffer::FrameBuffer(size_t width, size_t height)
	{
		m_Texture=CreateRef<Texture2D>(width, height);
		m_RendererId = CreateFrameBuffer(m_Texture);
	}
	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererId);
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