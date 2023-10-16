#include "KeyboardRecord.h"

AETHER_NAMESPACE_BEGIN
void KeyboardRecord::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyboardPressEvent>([this](KeyboardPressEvent& event) {return this->OnKeyPress(event);});
	dispatcher.Dispatch<KeyboardReleaseEvent>([this](KeyboardReleaseEvent& event) {return this->OnKeyRelease(event);});
}
bool KeyboardRecord::OnKeyPress(KeyboardPressEvent& event)
{
	m_Record[static_cast<int32_t>(event.GetCode())] = 1;
	return true;
}
bool KeyboardRecord::OnKeyRelease(KeyboardReleaseEvent& event)
{
	m_Record[static_cast<int32_t>(event.GetCode())] = 0;
	return true;
}
AETHER_NAMESPACE_END