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
			for (const auto& [entity, lc] : view.each())
			{
				Entity e = { entity,&scene };
				if (e.HasComponent<TransformComponent>())
				{
					auto& tc = e.GetComponent<TransformComponent>();
					lc.light.GetPosition() = tc.position;
				}
				Renderer3D::Submit(lc.light);
			}
		}
		
		
		// object pass
		{
			auto view = scene.GetAllEntitiesWith<VisualComponent>();
			for (const auto& [entity, vc] : view.each())
			{
				Entity e = { entity,&scene };
				Mat4 modelMatrix;
				if (e.HasComponent<TransformComponent>())
				{
					auto& tc = e.GetComponent<TransformComponent>();
					tc.CalculateMatrix();
					modelMatrix = tc.matrix;
				}
				for (size_t i = 0;i < vc.model->GetMeshes().size();i++)
				{
					Renderer3D::Submit(vc.model->GetMeshes()[i], vc.model->GetShaders()[i], modelMatrix);
				}
			}
		}
		
		Renderer3D::EndScene();
	}

}
