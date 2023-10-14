#pragma once
#include "../Core/Core.h"
#include "../Render/Camera.h"
#include "../Event/Event.h"
#include "../Event/KeyboardCode.h"
#include "../Event/KeyboardEvent.h"
AETHER_NAMESPACE_BEGIN
class CameraController
{
public:
	CameraController(const Camera& camera)
		:m_Camera(camera) {}
	void OnEvent(Event& event);
	bool OnKeyboardPress(KeyboardPressEvent& event);
	bool OnKeyboardRepeat(KeyboardRepeatEvent& event);
	void TranslateCamera(KeyboardCode code);
private:
	Camera m_Camera;
};
AETHER_NAMESPACE_END