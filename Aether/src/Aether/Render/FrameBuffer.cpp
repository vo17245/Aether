#include "FrameBuffer.h"
#include "../Core/Log.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Render/OpenGLApi.h"
#include "Aether/Render/Texture2D.h"
namespace Aether
{
	
	FrameBuffer::~FrameBuffer()
	{
		if(m_ColorRenderBuffer)
		{
			glDeleteRenderbuffers(1, &m_ColorRenderBuffer.value());
		}
		if(m_DepthRenderBuffer)
		{
			glDeleteRenderbuffers(1, &m_DepthRenderBuffer.value());
		}
		glDeleteFramebuffers(1, &m_RendererId);
	}
	void FrameBuffer::Bind()
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
	}
	void FrameBuffer::Unbind()
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
	
	FrameBuffer::FrameBuffer(const Spec& spec)
	{
		//create fbo
		GLCall(glGenFramebuffers(1, &m_RendererId));
		Bind();
		//create color buffer and attach to fbo
		if(spec.colorBufferSpec.storage==ColorBufferStorage::RENDER_BUFFER)
		{
			AETHER_ASSERT(false&&"not implemented");
		}
		else if(spec.colorBufferSpec.storage==ColorBufferStorage::TEXTURE)
		{
			Texture2D::Spec texSpec;
			texSpec.width=spec.width;
			texSpec.height=spec.height;
			if(spec.colorBufferSpec.format==ColorBufferFormat::RGB)
			{
				texSpec.format=Texture2D::Format::RGB;
			}
			else if(spec.colorBufferSpec.format==ColorBufferFormat::RGBA)
			{
				texSpec.format=Texture2D::Format::RGBA;
			}
			else
			{
				AETHER_ASSERT(false&&"unsupported color buffer format");
			}
			if(spec.colorBufferSpec.internalFormat==ColorBufferFormat::RGB)
			{
				texSpec.internalFormat=Texture2D::Format::RGB;
			}
			else if(spec.colorBufferSpec.internalFormat==ColorBufferFormat::RGBA)
			{
				texSpec.internalFormat=Texture2D::Format::RGBA;
			}
			else
			{
				AETHER_ASSERT(false&&"unsupported color buffer internal format");
			}
			if(spec.colorBufferSpec.dataType==ColorBufferScalarType::UBYTE)
			{
				texSpec.dataType=Texture2D::ScalarType::UBYTE;
			}
			else if(spec.colorBufferSpec.dataType==ColorBufferScalarType::FLOAT32)
			{
				texSpec.dataType=Texture2D::ScalarType::FLOAT32;
			}
			else
			{
				AETHER_ASSERT(false&&"unsupported color buffer scalar type");
			}
			m_ColorBuffer=CreateRef<Texture2D>(texSpec);
			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorBuffer->GetRendererId(), 0));

		}
		else
		{
			AETHER_ASSERT(false&&"unsupported color buffer storage");
		}
		//create depth buffer and attach to fbo
		uint32_t depthBuffer;
		GLCall(glGenRenderbuffers(1, &depthBuffer));
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, spec.width,
			spec.height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
		GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		m_DepthRenderBuffer=depthBuffer;
		// check if fbo is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			AETHER_DEBUG_LOG_ERROR("framebuffer is not complete");
			AETHER_ASSERT(false&&"framebuffer is not complete");
		}

	}
	

	
}