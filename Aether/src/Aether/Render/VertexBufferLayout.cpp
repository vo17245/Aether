#include "VertexBufferLayout.h"
#include "OpenGLApi.h"
#include "../Core/Log.h"
AETHER_NAMESPACE_BEGIN
VertexBufferLayout::VertexBufferLayout(VertexBufferLayout&& vbl)noexcept
	:m_Stride(vbl.m_Stride)
{
	m_Elements = std::move(vbl.m_Elements);
}

bool VertexBufferLayout::Use()
{
	for (size_t i=0;i<m_Elements.size();i++)
	{
		auto& element = m_Elements[i];
		if (element.m_Type == VertexBufferElementType::FLOAT)
		{
			glVertexAttribPointer(static_cast<GLuint>(i), 
				static_cast<GLint>(element.m_Count),
				static_cast<GLenum>(element.m_Type),
				GL_FALSE, 
				static_cast<GLsizei>(m_Stride), 
				(const void*)element.m_Offset);
			glEnableVertexAttribArray(static_cast<GLuint>(i));
		}
		else
		{
			AETHER_DEBUG_LOG_ERROR("Unknown VertexBufferElement Type");
			return false;
		}
	}
	return true;
}
AETHER_NAMESPACE_END