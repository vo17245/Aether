#include "CameraController.h"
#include "../Core/Input.h"
#include "Aether/Core/Input.h"
#include "Aether/Render/Transform.h"
#include <iostream>
namespace Aether
{
	void PerspectiveCameraController::OnUpdate(float ds)
	{
		Vec3 face = m_Camera.GetFace();
		face.normalize();
		Mat3 rotation = Transform::GetRotation(Vec3(0, 0, -1), face);
		Vec3 t_face = rotation * Vec3(0, 0, -1);
		Vec3 x_hat = rotation * Vec3(1, 0, 0);
		Vec3 y_hat = rotation * Vec3(0, 1, 0);
		Vec3 z_hat = rotation * Vec3(0, 0, 1);
		if (Input::Get().Pressed(KeyboardCode::KEY_W))
		{
			m_Camera.GetPosition() -= (z_hat * ds * m_Speed);
		}
		if (Input::Get().Pressed(KeyboardCode::KEY_S))
		{
			m_Camera.GetPosition() +=(z_hat * ds * m_Speed) ;
		}
		if (Input::Get().Pressed(KeyboardCode::KEY_A))
		{
			m_Camera.GetPosition() -=(x_hat * ds * m_Speed);
		}
		if (Input::Get().Pressed(KeyboardCode::KEY_D))
		{
			m_Camera.GetPosition() += (x_hat * ds * m_Speed);
		}
		if (Input::Get().Pressed(KeyboardCode::KEY_SPACE))
		{
			m_Camera.GetPosition() += (y_hat * ds * m_Speed);
		}
		if (Input::Get().Pressed(KeyboardCode::KEY_X))
		{
			m_Camera.GetPosition() -= (y_hat * ds * m_Speed);
		}
		HandleMouseMove(ds,rotation,face);
	}
	void PerspectiveCameraController::HandleMouseMove(float ds,
		const Mat3& rotation, const Vec3& face)
	{
		if (!Input::Get().Pressed(KeyboardCode::KEY_LEFT_CONTROL))
		{
			return;
		}
		Vec2i offset=Input::Get().GetMouseOffset();
		
		Vec3 x_hat=rotation*Vec3(1,0,0);
		Vec3 y_hat = (-face) .cross(x_hat);
		Vec4 h_face;
		h_face.head<3>() = face;
		h_face[3] = 1;
		h_face=Transform::Rotation(-x_hat,
		ds*m_MouseSpeed*offset.y())*
		Transform::Rotation(-y_hat,
		ds*m_MouseSpeed*offset.x())*
		h_face;
		if (std::abs(h_face.x()) < 0.01)
		{
			h_face.x() = 0;
		}
		if (std::abs(h_face.z()) < 0.01)
		{
			h_face.z() = 0;
		}
		if (std::abs(h_face.y()) < 0.01)
		{
			h_face.y() = 0;
		}
		m_Camera.GetFace() = h_face.head<3>();
	}
}

