#pragma once
#include "Event.h"
#include <string>
#include <vector>
#include "../Core/Core.h"
AETHER_NAMESPACE_BEGIN
class WindowFileDropEvent :public Event
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::WINDOW_FILE_DROP_EVENT; }
public:
	WindowFileDropEvent()
		:Event(EventType::WINDOW_FILE_DROP_EVENT) {}
	WindowFileDropEvent(const std::vector<std::string> paths)
		:Event(EventType::WINDOW_FILE_DROP_EVENT),m_Paths(paths){}
	WindowFileDropEvent(std::vector<std::string>&& paths)
		:Event(EventType::WINDOW_FILE_DROP_EVENT),m_Paths(paths){}
	WindowFileDropEvent(const WindowFileDropEvent& event)
		:Event(EventType::WINDOW_FILE_DROP_EVENT),m_Paths(event.m_Paths){}
	WindowFileDropEvent(WindowFileDropEvent&& event)noexcept
		:Event(EventType::WINDOW_FILE_DROP_EVENT),m_Paths(std::move(event.m_Paths)){}
	~WindowFileDropEvent(){}
	inline const std::vector<std::string>& GetPaths()const { return m_Paths; }
	inline void AddPath(const std::string& path) { m_Paths.emplace_back(path); }
private:
	std::vector<std::string> m_Paths;
};

class WindowResizeEvent :public Event
{
public:
	static constexpr inline const EventType GetStaticType() { return EventType::WINDOW_RESIZE_EVENT; }
public:
	WindowResizeEvent(size_t width, size_t height)
		:Event(EventType::WINDOW_RESIZE_EVENT), m_Width(width), m_Height(height) {}
	WindowResizeEvent(const WindowResizeEvent& event)
		:Event(EventType::WINDOW_RESIZE_EVENT), m_Width(event.m_Width), m_Height(event.m_Height) {}
	inline const size_t GetWidth()const { return m_Width; }
	inline const size_t GetHeight()const { return m_Height; }
private:
	size_t m_Width;
	size_t m_Height;
};
AETHER_NAMESPACE_END