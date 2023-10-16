#include "CameraController.h"

AETHER_NAMESPACE_BEGIN
void CameraController::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyboardPressEvent>([this](KeyboardPressEvent& event) {return this->OnKeyboardPress(event);});
	dispatcher.Dispatch< KeyboardRepeatEvent>([this](KeyboardRepeatEvent& event) {return this->OnKeyboardRepeat(event);});
	dispatcher.Dispatch<MouseScrollEvent>([this](MouseScrollEvent& event) {return this->OnMouseScroll(event);});
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
	return true;
}

void CameraController::TranslateCamera(KeyboardCode code)
{
	switch (code)
	{
	case KeyboardCode::KEY_A:
		m_Camera.Translate(-0.01f, 0.f, 0.f);
		break;
	case KeyboardCode::KEY_W:
		m_Camera.Translate(0.f,  0.f, 0.01f);
		break;
	case KeyboardCode::KEY_D:
		m_Camera.Translate(0.01f, 0.f, 0.f);
		break;
	case KeyboardCode::KEY_S:
		m_Camera.Translate(0.f,  0.f, -0.01f);
		break;
	}
	return;
}
AETHER_NAMESPACE_END


