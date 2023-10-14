#pragma once
#include "Event.h"

AETHER_NAMESPACE_BEGIN
enum class MouseButtonCode:int32_t
{
	MOUSE_BUTTON_LEFT	=   0,
	MOUSE_BUTTON_RIGHT	=   1,
	MOUSE_BUTTON_MIDDLE	=   2,
};
class MouseButtonEvent :public Event
{
public:
	MouseButtonEvent(MouseButtonCode code, EventType eventType)
		:m_Code(code), Event(eventType) {}
	inline MouseButtonCode GetCode() { return m_Code; }
private:
	MouseButtonCode m_Code;
};
class MouseButtonPressEvent :public MouseButtonEvent
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::MOUSE_BUTTON_PRESS_EVENT; }
public:
	MouseButtonPressEvent(MouseButtonCode code)
		:MouseButtonEvent(code, EventType::MOUSE_BUTTON_PRESS_EVENT) {}

};
class MouseButtonReleaseEvent :public MouseButtonEvent
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::MOUSE_BUTTON_RELEASE_EVENT; }
public:
	MouseButtonReleaseEvent(MouseButtonCode code)
		:MouseButtonEvent(code, EventType::MOUSE_BUTTON_RELEASE_EVENT) {}
};
class MouseScrollEvent:public Event
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::MOUSE_SCROLL_EVENT; }
public:
	MouseScrollEvent(float xOffset, float yOffset)
		:m_YOffset(yOffset), m_XOffset(xOffset) ,
		Event(EventType::MOUSE_SCROLL_EVENT) {}
	inline float GetXOffset()const { return m_XOffset; }
	inline float GetYOffset()const { return m_YOffset; }
private:
	float m_XOffset;
	float m_YOffset;
};


AETHER_NAMESPACE_END