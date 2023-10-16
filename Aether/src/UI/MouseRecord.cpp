#include "MouseRecord.h"
AETHER_NAMESPACE_BEGIN
void MouseRecord::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent& event) {return this->OnButtonPress(event);});
	dispatcher.Dispatch< MouseButtonReleaseEvent>([this](MouseButtonReleaseEvent& event) {return this->OnButtonRelease(event);});
	dispatcher.Dispatch< MousePositionEvent>([this](MousePositionEvent& event) {return this->OnMousePositionEvent(event);});
}
bool MouseRecord::OnButtonPress(MouseButtonPressEvent& event)
{
	m_Record[static_cast<int32_t>(event.GetCode())] = 1;
	return true;
}

bool MouseRecord::OnButtonRelease(MouseButtonReleaseEvent& event)
{
	m_Record[static_cast<int32_t>(event.GetCode())] = 0;
	return true;
}
bool MouseRecord::OnMousePositionEvent(MousePositionEvent& event)
{
	m_Offsets.emplace_back(event.GetPositionX() - m_LastPos.x(), event.GetPositionY() - m_LastPos.y());
	m_LastPos.x() = event.GetPositionX();
	m_LastPos.y() = event.GetPositionY();
	return true;
}
AETHER_NAMESPACE_END