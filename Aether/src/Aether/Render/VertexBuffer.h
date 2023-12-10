#pragma once
#include "../Core/Core.h"
#include <stdint.h>
#include "OpenGLApi.h"
namespace Aether
{

class VertexBuffer
{
public:
	VertexBuffer(const void* data,const size_t size);
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer(VertexBuffer&&) = delete;
	~VertexBuffer();
	void Bind();
	void Unbind();
	static Ref<VertexBuffer> Create(const void* data, const size_t size);
private:
	RendererId m_RendererId;
};
}
