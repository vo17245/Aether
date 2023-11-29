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
	static bool Pressed(KeyboardCode code);
	static void OnEvent(Event& e);
private:
	static std::unordered_map<KeyboardCode, bool> s_KeyboardRecord;
	static bool OnKeyboardPressEvent(KeyboardPressEvent& e);
	static bool OnKeyboardReleaseEvent(KeyboardReleaseEvent& e);
	static bool OnKeyboardRepeatEvent(KeyboardRepeatEvent& e);
};
}