#include "Input.h"
#include "Aether/Core/Log.h"
namespace Aether
{

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
    dispatcher.Dispatch<KeyboardPressEvent>(AETHER_BIND_FN(OnKeyboardPressEvent) );
    dispatcher.Dispatch<KeyboardReleaseEvent>(AETHER_BIND_FN(OnKeyboardReleaseEvent));
    dispatcher.Dispatch<KeyboardRepeatEvent>(AETHER_BIND_FN(OnKeyboardRepeatEvent));
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