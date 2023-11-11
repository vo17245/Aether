#include "Rectangle.h"
#include "../Render/VertexArray.h"
#include "../Render/VertexBuffer.h"
#include "../Render/VertexBufferLayout.h"
#include "../Render/IndexBuffer.h"
static float vertex[20] =
{
	-1,-1,0,0,0,
	1,-1,0,1,0,
	1,1,0,1,1,
	-1,1,0,0,1,
};
static uint32_t index[6] =
{
	0,1,2,
	2,3,0,
};
Aether::Rectangle::Rectangle()
{
	auto vb=VertexBuffer::Create(vertex, sizeof(float) * 20);
	auto vbl = VertexBufferLayout();
	vbl.Push<float>(3);
	vbl.Push<float>(2);
	auto va = VertexArray::Create();
	va->SetData(*vb, vbl);
	auto ib = CreateRef<IndexBuffer>(index,6);
	m_Mesh = CreateRef<Mesh>(va, vb,  ib);
}
