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
		Mesh(Ref<VertexArray> va, Ref<VertexBuffer>& vb, Ref<IndexBuffer>& ib)
			:m_VertexBuffer(vb), m_IndexBuffer(ib),
			m_VertexArray(va)
		{}
	public:
		inline Ref<VertexBuffer>& GetVertexBuffer() { return m_VertexBuffer; }
		inline Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }
		inline Ref<VertexArray>& GetVertexArray() { return m_VertexArray; }
	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<VertexArray> m_VertexArray;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
