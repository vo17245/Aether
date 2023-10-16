#pragma once
#include "../Event/Event.h"
#include <vector>
#include "../Event/KeyboardCode.h"
#include "../Event/KeyboardEvent.h"
AETHER_NAMESPACE_BEGIN
class KeyboardRecord
{
public:
	KeyboardRecord()
		:m_Record(1024,0) {}
	void OnEvent(Event& event);
	inline bool IsPressed(KeyboardCode code) { return m_Record[static_cast<int32_t>(code)]; }
public:
	bool OnKeyPress(KeyboardPressEvent& event);
	bool OnKeyRelease(KeyboardReleaseEvent& event);
private:
	std::vector<uint8_t> m_Record;
	
};
AETHER_NAMESPACE_END