#pragma once
#include <stdint.h>
#include "../Core/Core.h"
AETHER_NAMESPACE_BEGIN
enum class EventType :uint32_t
{
	UNKNOWN_EVENT=Bit(31),
	WINDOW_FILE_DROP_EVENT=Bit(0),
	WINDOW_RESIZE_EVENT=Bit(1),
	KEYBOARD_PRESS_EVENT=Bit(2),
	KEYBOARD_RELEASE_EVENT=Bit(3),
	MOUSE_SCROLL_EVENT=Bit(4),
	MOUSE_BUTTON_RELEASE_EVENT=Bit(5),
	MOUSE_BUTTON_PRESS_EVENT=Bit(6),
	KEYBOARD_REPEAT_EVENT = Bit(7),

};

class Event
{
	friend class EventDispatcher;
public:
	Event(EventType type,bool isHandle=false)
		:m_Type(type),m_IsHandle(isHandle){}
	inline bool IsOnCategory(EventType type)const { return static_cast<uint32_t>(type) & static_cast<uint32_t>(m_Type); }
	inline bool GetIsHandle()const { return m_IsHandle; }
	inline void SetHandle(bool status) { m_IsHandle = status; }
	inline EventType GetEventType()const { return m_Type; }
private:
	EventType m_Type;
	bool m_IsHandle;
};


class EventDispatcher
{
public:
	EventDispatcher(Event& event)
		: m_Event(event)
	{
	}

	
	template<typename T, typename F> 
	//requires std::invocable<F>&&std::same_as<std::invoke_result_t<F,T&>,bool>
	bool Dispatch(const F& func)
	{
		if (m_Event.GetEventType() == T::GetStaticType())
		{
			m_Event.m_IsHandle |= func(static_cast<T&>(m_Event));
			return true;
		}
		return false;
	}
private:
	Event& m_Event;
};
AETHER_NAMESPACE_END