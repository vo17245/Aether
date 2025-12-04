#pragma once
#include "Core/Math.h"
#include "EventBase.h"
#include "MouseButtonCode.h"
namespace Aether {
class MousePositionEvent : public EventBase<MousePositionEvent>
{
public:
    MousePositionEvent(const Vec2f& position) :
        m_Position(position)
    {
    }
    const Vec2f& GetPosition() const
    {
        return m_Position;
    }
    const char* GetNameImpl() const
    {
        return "MousePositionEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("MousePositionEvent: {0}, {1}", m_Position.x(), m_Position.y());
    }

private:
    Vec2f m_Position;
};
class MouseButtonPressedEvent : public EventBase<MouseButtonPressedEvent>
{
public:
    MouseButtonPressedEvent(MouseButtonCode keyCode):
        m_KeyCode(keyCode)
    {
    }
    const char* GetNameImpl() const
    {
        return "MouseButtonPressedEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("MouseButtonPressedEvent: {}", (uint32_t)m_KeyCode);
    }
    MouseButtonCode GetKeyCode() const
    {
        return m_KeyCode;
    }
private:
    MouseButtonCode m_KeyCode;

};
class MouseButtonReleasedEvent : public EventBase<MouseButtonReleasedEvent>
{
public:
    MouseButtonReleasedEvent(MouseButtonCode keyCode) :
        m_KeyCode(keyCode)
    {
    }
    const char* GetNameImpl() const
    {
        return "MouseButtonReleasedEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("MouseButtonReleasedEvent: {}",(uint32_t) m_KeyCode);
    }
    MouseButtonCode GetKeyCode() const
    {
        return m_KeyCode;
    }
private:
    MouseButtonCode m_KeyCode;
};
} // namespace Aether