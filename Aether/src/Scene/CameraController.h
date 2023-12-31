#pragma once
#include "../Render/Camera.h"
namespace Aether
{
	class PerspectiveCameraController
	{
	public:
		PerspectiveCameraController(float fovy, float zNear, float zFar, float aspectRatio)
			:m_Camera(fovy, zNear, zFar, aspectRatio) {}
		void OnUpdate(float ds);
		inline PerspectiveCamera& GetCamera() { return m_Camera;}
	private:
		float m_Speed = 3;
		PerspectiveCamera m_Camera;
	};
}