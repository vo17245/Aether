#pragma once
#include "../Event/Event.h"
#include "../Event/KeyboardCode.h"
#include "../Event/KeyboardEvent.h"
#include <unordered_map>
#include "../Event/MouseEvent.h"

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
private:
	Input() = default;
	std::unordered_map<KeyboardCode, bool> m_KeyboardRecord;
	std::vector<MousePositionEvent> m_MouseTrace;
	bool OnKeyboardPressEvent(KeyboardPressEvent& e);
	bool OnKeyboardReleaseEvent(KeyboardReleaseEvent& e);
	bool OnKeyboardRepeatEvent(KeyboardRepeatEvent& e);
	bool OnMousePositionEvent(MousePositionEvent& e);

	void OnLoopEnd()
	{
		m_MouseTrace.clear();
	}
};
}