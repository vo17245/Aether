#pragma once
#include "../Event/Event.h"
namespace Aether
{
	class Layer
	{
	public:
		Layer() {}
		virtual ~Layer() {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}
		virtual void OnUpdate(float sec) {}
		virtual void OnLoopEnd() {}
		virtual void OnLoopBegin() {}
	};
}