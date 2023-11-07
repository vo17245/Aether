#pragma once

#include "Aether/ImGui/imgui.h"
#include "Aether/ImGui/imgui_impl_glfw.h"
#include "Aether/ImGui/imgui_impl_opengl3.h"
#include "Aether/Event/Event.h"
#include <vector>
#include "Aether/Core/Core.h"
#define RESOURCE(x) ("../../Resource/" x)
namespace Test
{
	
	class Test
	{
	public:
		Test() {}
		virtual ~Test() {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Aether::Event& event) {}
		virtual void OnUpdate(float sec) {}
		virtual void OnLoopEnd() {}
		virtual void OnLoopBegin() {}
	};
	
}
