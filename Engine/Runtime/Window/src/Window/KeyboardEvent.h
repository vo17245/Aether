#pragma once
#include <cstdint>
#include <format>
#include <variant>
#include <string>
#include "KeyboardCode.h"
#include "EventBase.h"
namespace Aether {
class KeyboardPressEvent : public EventBase<KeyboardPressEvent>
{
    friend class EventBase;

public:
    KeyboardPressEvent(KeyboardCode code) :
        m_Code(code)
    {
    }
    KeyboardCode GetCode() const
    {
        return m_Code;
    }

private:
    const char* GetNameImpl() const
    {
        return "KeyboardPressEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("KeyboardPressEvent: {0}", KeyboardCodeToString(m_Code));
    }
    KeyboardCode m_Code;
};

class KeyboardReleaseEvent : public EventBase<KeyboardReleaseEvent>
{
    friend class EventBase;

public:
    KeyboardReleaseEvent(KeyboardCode code) :
        m_Code(code)
    {
    }
    KeyboardCode GetCode() const
    {
        return m_Code;
    }

private:
    const char* GetNameImpl() const
    {
        return "KeyboardReleaseEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("KeyboardReleaseEvent: {0}", KeyboardCodeToString(m_Code));
    }
    KeyboardCode m_Code;
};
class KeyboardRepeatEvent : public EventBase<KeyboardRepeatEvent>
{
    friend class EventBase;

public:
    KeyboardRepeatEvent(KeyboardCode code) :
        m_Code(code)
    {
    }
    KeyboardCode GetCode() const
    {
        return m_Code;
    }

private:
    const char* GetNameImpl() const
    {
        return "KeyboardRepeatEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("KeyboardRepeatEvent: {0}", KeyboardCodeToString(m_Code));
    }
    KeyboardCode m_Code;
};
} // namespace Aether