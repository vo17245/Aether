#include "VertexBuffer.h"
#include "OpenGLApi.h"
AETHER_NAMESPACE_BEGIN
VertexBuffer::VertexBuffer(const void* data,const size_t size)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
}

void VertexBuffer::Bind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
}

void VertexBuffer::Unbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
AETHER_NAMESPACE_END