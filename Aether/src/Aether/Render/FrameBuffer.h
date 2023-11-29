#pragma once
#include "OpenGLApi.h"
#include "Texture2D.h"
namespace Aether
{
	class FrameBuffer
	{
	public:
		FrameBuffer(Ref<Texture2D>& tex);
		FrameBuffer(size_t width, size_t height);
		~FrameBuffer();
		void Bind();
		void Unbind();
		inline RendererId GetRendererId() { return m_RendererId; }
		inline Ref<Texture2D>& GetTexture() { return m_Texture; }
		inline const Ref<Texture2D>& GetTexture()const { return m_Texture; }
	private:
		RendererId m_RendererId;
		Ref<Texture2D> m_Texture;
		RendererId m_RenderBuferId;
	};
}