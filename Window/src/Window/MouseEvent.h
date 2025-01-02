#pragma once
#include "Core/Math.h"
#include "EventBase.h"
namespace Aether {
class MousePositionEvent : public EventBase<MousePositionEvent>
{
public:
    MousePositionEvent(const Vec2& position) :
        m_Position(position)
    {
    }
    Vec2 GetPosition() const
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
    Vec2 m_Position;
};

} // namespace Aether