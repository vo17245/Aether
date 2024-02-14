#pragma once
#include "../Render/Camera.h"
#include "Aether/Core/Math.h"
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
		float m_MouseSpeed=Math::PI/16;
		PerspectiveCamera m_Camera;
		void HandleMouseMove(float ds,const Mat3& rotation,const Vec3& face);
	};
}