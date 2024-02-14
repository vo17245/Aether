#include "Input.h"
#include "Aether/Core/Log.h"
namespace Aether
{

bool Input::Pressed(KeyboardCode code)
{
    auto iter = m_KeyboardRecord.find(code);
    if (iter != m_KeyboardRecord.end())
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
    dispatcher.Dispatch<MousePositionEvent>(
        AETHER_BIND_FN(OnMousePositionEvent));
}

bool Input::OnKeyboardPressEvent(KeyboardPressEvent& e)
{
    m_KeyboardRecord[e.GetCode()] = true;
    return true;
}

bool Input::OnKeyboardReleaseEvent(KeyboardReleaseEvent& e)
{

    m_KeyboardRecord[e.GetCode()] = false;
    return true;
}

bool Input::OnKeyboardRepeatEvent(KeyboardRepeatEvent& e)
{
    m_KeyboardRecord[e.GetCode()] = true;
    return true;
}
bool Input::OnMousePositionEvent(MousePositionEvent& e)
{
    if(!m_MouseTrace.empty())
    {
        m_MouseOffset+=Vec2i(e.GetPositionX()-m_MouseTrace.back().GetPositionX(),
        e.GetPositionY()-m_MouseTrace.back().GetPositionY());
    }
    else
    {
        m_MouseOffset += Vec2i(e.GetPositionX() - m_LastMousePos.x(),
            e.GetPositionY() - m_LastMousePos.y());
    }
    m_MouseTrace.emplace_back(e);
    return true;
}
}