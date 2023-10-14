#include "VertexArray.h"
#include "OpenGLApi.h"
AETHER_NAMESPACE_BEGIN
VertexArray::VertexArray()
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
AETHER_NAMESPACE_END