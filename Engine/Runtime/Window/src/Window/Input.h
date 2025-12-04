#pragma once
#include "KeyboardCode.h"
#include <unordered_map>
#include <variant>
#include "Event.h"
#include "MouseEvent.h"
#include "Window/Event.h"
namespace Aether {
class Keyboard
{
public:
    Keyboard() = default;
    ~Keyboard() = default;
    bool IsPressed(KeyboardCode key) const
    {
        auto iter = m_IsPressed.find(key);
        if (iter == m_IsPressed.end())
        {
            return false;
        }
        return iter->second;
    }
    void OnEvent(Event& event)
    {
        if (std::holds_alternative<KeyboardPressEvent>(event))
        {
            auto key = std::get<KeyboardPressEvent>(event).GetCode();
            m_IsPressed[key] = true;
        }
        else if (std::holds_alternative<KeyboardReleaseEvent>(event))
        {
            auto key = std::get<KeyboardReleaseEvent>(event).GetCode();
            m_IsPressed[key] = false;
        }
    }

private:
    std::unordered_map<KeyboardCode, bool> m_IsPressed;
};
class Mouse
{
public:
    Mouse() = default;
    ~Mouse() = default;
    void OnEvent(Event& event)
    {
        if (std::holds_alternative<MousePositionEvent>(event))
        {
            MousePositionEvent& e = std::get<MousePositionEvent>(event);
            m_CurPos = e.GetPosition();
        }
    }
    Vec2f GetCurPos() const
    {
        return m_CurPos;
    }

private:
    Vec2f m_CurPos;
};
class Input
{
public:
    Input() = default;
    ~Input() = default;
    void OnEvent(Event& event)
    {
        m_Keyboard.OnEvent(event);
        m_Mouse.OnEvent(event);
    }
    bool IsKeyPressed(KeyboardCode key) const
    {
        return m_Keyboard.IsPressed(key);
    }
    Vec2f MouseCurPos() const
    {
        return m_Mouse.GetCurPos();
    }

private:
    Keyboard m_Keyboard;
    Mouse m_Mouse;
};
} // namespace Aether