#include "Input.h"

namespace Aether
{
    std::unordered_map<KeyboardCode, bool> Input::s_KeyboardRecord;

bool Input::Pressed(KeyboardCode code)
{
    auto iter = s_KeyboardRecord.find(code);
    if (iter != s_KeyboardRecord.end())
    {
        return iter->second;
    }
    return false;
}

void Input::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyboardPressEvent>(OnKeyboardPressEvent);
    dispatcher.Dispatch<KeyboardReleaseEvent>(OnKeyboardReleaseEvent);
    dispatcher.Dispatch<KeyboardRepeatEvent>(OnKeyboardRepeatEvent);
}

bool Input::OnKeyboardPressEvent(KeyboardPressEvent& e)
{
    s_KeyboardRecord[e.GetCode()] = true;
    return true;
}

bool Input::OnKeyboardReleaseEvent(KeyboardReleaseEvent& e)
{
    s_KeyboardRecord[e.GetCode()] = false;
    return true;
}

bool Input::OnKeyboardRepeatEvent(KeyboardRepeatEvent& e)
{
    s_KeyboardRecord[e.GetCode()] = true;
    return true;
}
}