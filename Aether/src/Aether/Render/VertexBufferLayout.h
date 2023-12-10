#pragma once
#include "../Core/Core.h"
#include <stdint.h>
#include <assert.h>
#include <vector>

namespace Aether
{

enum class VertexBufferElementType:int32_t
{
	FLOAT=0x1406  //#define GL_FLOAT 0x1406
};

class VertexBufferElement
{
public:
	VertexBufferElement(VertexBufferElementType type,size_t count,size_t offset)
		:m_Type(type),m_Count(count),m_Offset(offset) {}
	VertexBufferElementType m_Type;
	size_t m_Count;//这个元素中数据的数量
	size_t m_Offset;//这个元素在一个顶点数据中的偏移

};
class VertexBufferLayout
{
public:
	VertexBufferLayout()
		:m_Stride(0) {}
	VertexBufferLayout(VertexBufferLayout&& vbl)noexcept;
	VertexBufferLayout(const VertexBufferLayout& vbl)
		:m_Stride(vbl.m_Stride), m_Elements(vbl.m_Elements){}
	template<typename T>
	void Push(size_t n) { assert(false && "Unknown VertexBufferElement Type"); }
	bool Use();
private:
	std::vector<VertexBufferElement> m_Elements;
	size_t m_Stride;// bytes of a vertex
};
template<>
inline void VertexBufferLayout::Push<float>(size_t n)
{
	m_Elements.push_back(VertexBufferElement(VertexBufferElementType::FLOAT, n, m_Stride));
	m_Stride += n * sizeof(float);
}
}//namespace Aether

