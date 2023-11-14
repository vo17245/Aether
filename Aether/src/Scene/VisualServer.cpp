#include "VisualServer.h"
#include "Component.h"
#include "../Render/Renderer3D.h"
namespace Aether
{
	void VisualServer::Render(Scene& scene, Camera& camera)
	{
		Renderer3D::BeginScene(&camera);
		//add lights
		{
			auto view = scene.GetAllEntitiesWith<PointLightComponent>();
			for (auto& [entity, lc] : view.each())
			{
				Renderer3D::Submit(lc.Light);
			}
		}
		
		
		// object pass
		{
			auto view = scene.GetAllEntitiesWith<VisualComponent>();
			for (auto& [entity, vc] : view.each())
			{
				Entity e = { entity,&scene };
				Mat4 modelMatrix;
				if (e.HasComponent<TransformComponent>())
				{
					auto& tc = e.GetComponent<TransformComponent>();
					tc.CalculateMatrix();
					modelMatrix = tc.Matrix;
				}
				for (size_t i = 0;i < vc.Meshes.size();i++)
				{
					Renderer3D::Submit(vc.Meshes[i], vc.Shaders[i], modelMatrix);
				}
			}
		}
		
		Renderer3D::EndScene();
	}

}
