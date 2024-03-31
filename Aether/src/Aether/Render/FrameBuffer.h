#pragma once
#include "Aether/Render/Texture2D.h"
#include "OpenGLApi.h"
#include "Texture2D.h"
#include <stdint.h>
namespace Aether
{
	class FrameBuffer
	{
	public:
		enum ColorBufferFormat:int32_t
		{
			RGB=0,
			RGBA=1,
		};
		enum ColorBufferScalarType:int32_t
		{
			UBYTE=0,
			FLOAT32=1,
		};
		enum ColorBufferStorage:int32_t
		{
			TEXTURE=0,
			RENDER_BUFFER=1,
		};
		struct ColorBufferSpec
		{
			
			ColorBufferFormat format;//input data format in cpu 
			ColorBufferFormat internalFormat;//data format in gpu
			ColorBufferScalarType dataType;
			ColorBufferStorage storage;
		};
		struct Spec
		{
			size_t width;
			size_t height;
			ColorBufferSpec colorBufferSpec;
		};
	public:
		FrameBuffer(const Spec& spec);
		~FrameBuffer();
		void Bind();
		void Unbind();
		inline RendererId GetRendererId() { return m_RendererId; }
		inline Ref<Texture2D>& GetColorBuffer() { return m_ColorBuffer; }
		inline const Ref<Texture2D>& GetColorBuffer()const { return m_ColorBuffer; }
		inline const bool HasColorBuffer(){return bool(m_ColorBuffer);}
		inline const bool HasDepthBuffer(){return bool(m_DepthBuffer);}
		inline const bool HasColorRenderBuffer(){return m_ColorRenderBuffer.has_value();}
		inline const bool HasDepthRenderBuffer(){return m_DepthRenderBuffer.has_value();}
		inline const Ref<Texture2D>& GetDepthBuffer()const { return m_DepthBuffer; }
		inline Ref<Texture2D>& GetDepthBuffer() { return m_DepthBuffer; }
	private:
		RendererId m_RendererId;
		Ref<Texture2D> m_ColorBuffer;
		Ref<Texture2D> m_DepthBuffer;
		std::optional<RendererId> m_ColorRenderBuffer;
		std::optional<RendererId> m_DepthRenderBuffer;

	};
}