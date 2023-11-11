#pragma once
#include "../Render/Mesh.h"
namespace Aether
{
	//-1 ~ 1 Rectangle
	class Rectangle
	{
	public:
		Rectangle();
		inline Ref<Mesh>& GetMesh() { return m_Mesh; }
	private:
		Ref<Mesh> m_Mesh;
		
	};
}