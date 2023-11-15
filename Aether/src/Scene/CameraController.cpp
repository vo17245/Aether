#include "CameraController.h"
#include "../Core/Input.h"
void Aether::PerspectiveCameraController::OnUpdate(float ds)
{
	if (Input::Pressed(KeyboardCode::KEY_W))
	{
		m_Camera.GetPosition().z() -= ds * m_Speed;
	}
	if (Input::Pressed(KeyboardCode::KEY_S))
	{
		m_Camera.GetPosition().z() += ds * m_Speed;
	}
	if (Input::Pressed(KeyboardCode::KEY_A))
	{
		m_Camera.GetPosition().x() -= ds * m_Speed;
	}
	if (Input::Pressed(KeyboardCode::KEY_D))
	{
		m_Camera.GetPosition().x() += ds * m_Speed;
	}
	if (Input::Pressed(KeyboardCode::KEY_SPACE))
	{
		m_Camera.GetPosition().y() += ds * m_Speed;
	}
	if (Input::Pressed(KeyboardCode::KEY_X))
	{
		m_Camera.GetPosition().y() -= ds * m_Speed;
	}
}
