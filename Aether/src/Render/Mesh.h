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
		Mesh(Ref<VertexBuffer>& vb,Ref<VertexArray> va, Ref<IndexBuffer>& ib, Ref<Shader>& shader)
			:m_VertexBuffer(vb), m_IndexBuffer(ib), m_Shader(shader),
			m_VertexArray(va)
		{}
	public:
		inline Ref<VertexBuffer>& GetVertexBuffer() { return m_VertexBuffer; }
		inline Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }
		inline Ref<Shader>& GetShader() { return m_Shader; }
		inline Ref<VertexArray>& GetVertexArray() { return m_VertexArray; }
	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<VertexArray> m_VertexArray;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Shader> m_Shader;
	};
}
