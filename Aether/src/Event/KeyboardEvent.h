#pragma once
#include "Event.h"
#include "KeyboardCode.h"
AETHER_NAMESPACE_BEGIN


class KeyboardEvent:public Event
{
public:
	KeyboardEvent(KeyboardCode code,EventType eventType)
		:m_Code(code),Event(eventType) {}
	inline const KeyboardCode GetCode()const { return m_Code; }
private:
	KeyboardCode m_Code;
};

class KeyboardPressEvent:public KeyboardEvent
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::KEYBOARD_PRESS_EVENT; }
public:
	KeyboardPressEvent(KeyboardCode code)
		:KeyboardEvent(code, EventType::KEYBOARD_PRESS_EVENT) {}

};
class KeyboardReleaseEvent :public KeyboardEvent
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::KEYBOARD_RELEASE_EVENT; }
public:
	KeyboardReleaseEvent(KeyboardCode code)
		:KeyboardEvent(code, EventType::KEYBOARD_RELEASE_EVENT) {}
};
class KeyboardRepeatEvent :public KeyboardEvent
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::KEYBOARD_REPEAT_EVENT; }
public:
	KeyboardRepeatEvent(KeyboardCode code)
		:KeyboardEvent(code, EventType::KEYBOARD_REPEAT_EVENT) {}
};
AETHER_NAMESPACE_END