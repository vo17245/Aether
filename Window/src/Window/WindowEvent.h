#pragma once
#include <cstdint>
#include <format>
#include <variant>
#include <string>
#include "KeyboardCode.h"
#include "EventBase.h"
namespace Aether {
class WindowResizeEvent : public EventBase<WindowResizeEvent>
{
    friend class EventBase;

public:
    WindowResizeEvent(uint32_t width, uint32_t height) :
        m_Width(width), m_Height(height)
    {
    }
    uint32_t GetWidth() const
    {
        return m_Width;
    }
    uint32_t GetHeight() const
    {
        return m_Height;
    }

private:
    uint32_t m_Width;
    uint32_t m_Height;

public:
    const char* GetNameImpl() const
    {
        return "WindowResizeEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("WindowResizeEvent: {0}, {1}", m_Width, m_Height);
    }
};

class FrameBufferResizeEvent : public EventBase<FrameBufferResizeEvent>
{
    friend class EventBase;

public:
    FrameBufferResizeEvent(uint32_t width, uint32_t height) :
        m_Width(width), m_Height(height)
    {
    }
    uint32_t GetWidth() const
    {
        return m_Width;
    }
    uint32_t GetHeight() const
    {
        return m_Height;
    }

private:
    uint32_t m_Width;
    uint32_t m_Height;

private:
    const char* GetNameImpl() const
    {
        return "FrameBufferResizeEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("FrameBufferResizeEvent: {0}, {1}", m_Width, m_Height);
    }
};
class CharacterInputEvent : public EventBase<CharacterInputEvent>
{
    friend class EventBase;

public:
    CharacterInputEvent(uint32_t code) :
        m_Code(code)
    {
    }
    uint32_t GetCode() const
    {
        return m_Code;
    }
    const char* GetNameImpl() const
    {
        return "CharacterInputEvent";
    }
    std::string ToStringImpl() const
    {
        return std::format("CharacterInputEvent: {0}", m_Code);
    }

private:
    uint32_t m_Code;
};
} // namespace Aether