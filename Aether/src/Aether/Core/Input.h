#pragma once
#include "../Event/Event.h"
#include "../Event/KeyboardCode.h"
#include "../Event/KeyboardEvent.h"
#include <unordered_map>
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
private:
	Input() = default;
	std::unordered_map<KeyboardCode, bool> s_KeyboardRecord;
	bool OnKeyboardPressEvent(KeyboardPressEvent& e);
	bool OnKeyboardReleaseEvent(KeyboardReleaseEvent& e);
	bool OnKeyboardRepeatEvent(KeyboardRepeatEvent& e);
};
}