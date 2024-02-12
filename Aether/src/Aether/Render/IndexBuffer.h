#pragma once
#include "../Core/Core.h"
#include <stdint.h>
namespace Aether
{

class IndexBuffer
{
public:
	IndexBuffer(const std::byte* buf, const size_t len);
	~IndexBuffer();
	void Bind()const;
	void Unbind()const;
	static Ref<IndexBuffer> Create(const std::byte* buf,const size_t len);
private:
	uint32_t m_RendererId;
};
}
