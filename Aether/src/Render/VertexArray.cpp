#include "VertexArray.h"
#include "OpenGLApi.h"
namespace Aether
{
	VertexArray::VertexArray()
		:m_RendererId(0)
	{
		GLCall(glGenVertexArrays(1, &m_RendererId));
		GLCall(glBindVertexArray(m_RendererId));
	}

	VertexArray::~VertexArray()
	{
		GLCall(glDeleteVertexArrays(1, &m_RendererId));
	}

	void VertexArray::Bind()const
	{
		GLCall(glBindVertexArray(m_RendererId));
	}

	void VertexArray::Unbind()const
	{
		GLCall(glBindVertexArray(0));
	}

	void VertexArray::SetData(VertexBuffer& vb, VertexBufferLayout& vbl)
	{
		Bind();
		vb.Bind();
		vbl.Use();
	}
	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<VertexArray>();
	}
}//namespace Aether