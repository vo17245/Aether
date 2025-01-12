#pragma once
#include "Event.h"
namespace Aether {

// EventBase 多态函数

class EventBaseIsHandledVisitor
{
public:
    template <typename T>
    bool operator()(const T& event)
    {
        return event.IsHandled();
    }
    
};
template <>
inline bool EventBaseIsHandledVisitor::operator()(const std::monostate& event)
{
    assert(false&&"EventBaseOperator: IsHandled: invalid event type");
    return false;
}
inline bool EventBaseIsHandled(const Event& event)
{
    return std::visit(EventBaseIsHandledVisitor{}, event);
}
class EventBaseToStringVisitor
{
public:
    template <typename T>
    std::string operator()(const T& event)
    {
        return event.ToString();
    }
    
};
template <>
inline std::string EventBaseToStringVisitor::operator()(const std::monostate& event)
{
    assert(false&&"EventBaseOperator: ToString: invalid event type");
    return "EmptyEvent";
}
inline std::string EventBaseToString(const Event& event)
{
    return std::visit(EventBaseToStringVisitor{}, event);
}

class EventBaseHandleVisitor
{
public:
    template <typename T>
    void operator()(T& event)
    {
        event.Handle();
    }
};
template <>
inline void EventBaseHandleVisitor::operator()(std::monostate& event)
{
    assert(false&&"EventBaseOperator: Handle: invalid event type");
}
inline void EventBaseHandle(Event& event)
{
    return std::visit(EventBaseHandleVisitor{}, event);
}
} // namespace Aether