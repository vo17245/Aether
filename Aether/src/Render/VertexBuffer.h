#pragma once
#include "../Core/Core.h"
#include <stdint.h>
AETHER_NAMESPACE_BEGIN
class VertexBuffer
{
public:
	VertexBuffer(const void* data,const size_t size);
	~VertexBuffer();
	void Bind();
	void Unbind();
	static Ref<VertexBuffer> Create(const void* data, const size_t size);
private:
	uint32_t m_RendererId;
};
AETHER_NAMESPACE_END