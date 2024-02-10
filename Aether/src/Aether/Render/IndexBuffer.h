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
private:
	uint32_t m_RendererId;
};
}
