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
	private:
		PerspectiveCamera m_Camera;
	};
}