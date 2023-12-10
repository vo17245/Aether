#pragma once
#include "../Core/Core.h"
#include <stdint.h>
namespace Aether
{

class IndexBuffer
{
public:
	IndexBuffer(const uint32_t* buf, const size_t count);
	~IndexBuffer();
	void Bind()const;
	void Unbind()const;
	inline const size_t GetCount()const { return m_Count; }
private:
	uint32_t m_RendererId;
	size_t m_Count;
};
}
