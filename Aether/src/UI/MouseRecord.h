#pragma once
#include "../Event/Event.h"
#include <vector>
#include <Eigen/Core>
#include "../Event/MouseEvent.h"
AETHER_NAMESPACE_BEGIN
class MouseRecord
{
public:
	MouseRecord(Eigen::Vector2d curPos)
		:m_Record(64, 0), m_LastPos(curPos) {}
	void OnEvent(Event& event);
	inline bool IsPressed(MouseButtonCode code) { return m_Record[static_cast<uint32_t>(code)]; }
public:
	bool OnButtonPress(MouseButtonPressEvent& event);
	bool OnButtonRelease(MouseButtonReleaseEvent& event);
	bool OnMousePositionEvent(MousePositionEvent& event);
	const std::vector<Eigen::Vector2d>& GetOffsets()const { return m_Offsets; }
	std::vector<Eigen::Vector2d>& GetOffsets() { return m_Offsets; }
	void ClearOffsets() { m_Offsets.clear(); }

private:
	std::vector<uint8_t> m_Record;
	Eigen::Vector2d m_LastPos;
	std::vector<Eigen::Vector2d> m_Offsets;
};
AETHER_NAMESPACE_END