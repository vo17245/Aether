#include "CameraController.h"

AETHER_NAMESPACE_BEGIN
void CameraController::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyboardPressEvent>([this](KeyboardPressEvent& event) {return this->OnKeyboardPress(event);});
	dispatcher.Dispatch< KeyboardRepeatEvent>([this](KeyboardRepeatEvent& event) {return this->OnKeyboardRepeat(event);});
}

bool CameraController::OnKeyboardPress(KeyboardPressEvent& event)
{
	TranslateCamera(event.GetCode());
	return true;
}

bool CameraController::OnKeyboardRepeat(KeyboardRepeatEvent& event)
{
	TranslateCamera(event.GetCode());
	return true;
}

bool CameraController::OnMouseScroll(MouseScrollEvent& event)
{
	if (event.GetYOffset() > 0)
	{
		m_Camera.Scale(1.25f, 1.25f, 1.25f);
	}
	else
	{
		m_Camera.Scale(0.8f, 0.8f, 0.8f);
	}
	return false;
}

void CameraController::TranslateCamera(KeyboardCode code)
{
	switch (code)
	{
	case KeyboardCode::KEY_A:
		m_Camera.Translate(0.01f, 0.f, 0.f);
		goto UPDATE_CAMERA;
		break;
	case KeyboardCode::KEY_W:
		m_Camera.Translate(0.f, 0.01f, 0.f);
		goto UPDATE_CAMERA;
		break;
	case KeyboardCode::KEY_D:
		m_Camera.Translate(-0.01f, 0.f, 0.f);
		goto UPDATE_CAMERA;
		break;
	case KeyboardCode::KEY_S:
		m_Camera.Translate(0.f, -0.01f, 0.f);
		goto UPDATE_CAMERA;
		break;
	}
	return;
UPDATE_CAMERA:
	m_Camera.CalculateViewMatrix();
	m_Camera.CalculateCameraMatrix();
	return;
}
AETHER_NAMESPACE_END


