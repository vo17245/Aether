#pragma once
#include "../Core/Core.h"
#include "Aether/Render/OpenGLApi.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "OpenGLApi.h"
namespace Aether
{
class VertexArray
{
public:
	VertexArray();
	~VertexArray();
	VertexArray(const VertexArray&) = delete;
	VertexArray(VertexArray&&) = delete;
	void Bind()const;
	void Unbind()const;
	void BindData(VertexBuffer& vb,VertexBufferLayout& vbl);
	static Ref<VertexArray> Create();
private:
	RendererId m_RendererId;
};
}//namespace Aether