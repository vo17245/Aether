#pragma once
#include "../Event/Event.h"
#include "../Event/KeyboardCode.h"
#include "../Event/KeyboardEvent.h"
#include <unordered_map>
#include "../Event/MouseEvent.h"
#include "Aether/Core/Math.h"
namespace Aether
{

class Input
{
	friend class Application;
public:
	bool Pressed(KeyboardCode code);
	void OnEvent(Event& e);
	
	static Input& Get()
	{
		static Input input;
		return input;
	}
	const std::vector<MousePositionEvent>& GetMouseTrace()
	{
		return m_MouseTrace;
	}
	Vec2i GetMouseOffset()
	{
		return m_MouseOffset;
	}
private:
	Input() = default;
	std::unordered_map<KeyboardCode, bool> m_KeyboardRecord;
	std::vector<MousePositionEvent> m_MouseTrace;
	Vec2i m_MouseOffset=Vec2i(0,0);
	Vec2i m_LastMousePos=Vec2i(0,0);
	bool OnKeyboardPressEvent(KeyboardPressEvent& e);
	bool OnKeyboardReleaseEvent(KeyboardReleaseEvent& e);
	bool OnKeyboardRepeatEvent(KeyboardRepeatEvent& e);
	bool OnMousePositionEvent(MousePositionEvent& e);

	void OnLoopEnd()
	{
		if (!m_MouseTrace.empty())
		{
			m_LastMousePos.x() = m_MouseTrace.back().GetPositionX();
			m_LastMousePos.y() = m_MouseTrace.back().GetPositionY();

		}
		m_MouseTrace.clear();
		m_MouseOffset=Vec2i(0,0);
	}
};
}