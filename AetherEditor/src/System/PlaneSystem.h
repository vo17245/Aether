#pragma once
#include "Aether/Render.h"
#include "Aether/Scene.h"

namespace Aether
{
	namespace Editor
	{
		//warning: only one entity hold PlaneComponent in one scene
		struct PlaneComponent
		{
			PlaneComponent();
			float m_Scaling;// x_hat z_hat vector scaling
			float m_Size;// size of plane
			float m_LineWidth;
			Ref<Model> m_XzPlaneQuad;
			Ref<Shader> m_Shader;
		};
		class PlaneSystem
		{
		public:
			PlaneSystem() = default;
			~PlaneSystem() = default;
			void OnRender(Scene& scene);
		};
	}
}