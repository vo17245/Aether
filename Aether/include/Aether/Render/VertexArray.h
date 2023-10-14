#pragma once
#include "../Core/Core.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
AETHER_NAMESPACE_BEGIN
class VertexArray
{
public:
	VertexArray();
	~VertexArray();
	void Bind()const;
	void Unbind()const;
	void SetData(VertexBuffer& vb,VertexBufferLayout& vbl);
private:
	uint32_t m_RendererId;
};
AETHER_NAMESPACE_END