#pragma once
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "../Core/Core.h"
#include "Shader.h"
namespace Aether
{
	class Mesh
	{
	public:
		Mesh(Ref<VertexBuffer>& vb, Ref<IndexBuffer>& ib, Ref<Shader>& shader)
			:m_VertexBuffer(vb), m_IndexBuffer(ib), m_Shader(shader) {}
	public:
		inline Ref<VertexBuffer>& GetVertexBuffer() { return m_VertexBuffer; }
		inline Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }
		inline Ref<Shader>& GetShader() { return m_Shader; }
	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Shader> m_Shader;
	};
}
