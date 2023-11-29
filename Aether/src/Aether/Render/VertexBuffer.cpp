#include "VertexBuffer.h"
#include "OpenGLApi.h"
AETHER_NAMESPACE_BEGIN
VertexBuffer::VertexBuffer(const void* data,const size_t size)
    :m_RendererId(0)
{
    GLCall(glGenBuffers(1, &m_RendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_RendererId));
}

void VertexBuffer::Bind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
}

void VertexBuffer::Unbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
Ref<VertexBuffer> VertexBuffer::Create(const void* data, const size_t size)
{
    return CreateRef<VertexBuffer>(data,size);
}
AETHER_NAMESPACE_END