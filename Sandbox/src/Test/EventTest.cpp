#include "EventTest.h"

#include "Aether/Core/Log.h"
#include "Aether/Core/Core.h"
#include "Aether/Event/MouseEvent.h"
void Test::EventTest::OnEvent(Aether::Event& event)
{
	Aether::EventDispatcher dispatcher(event);
	auto onKeyboardRelease = [](Aether::KeyboardReleaseEvent& event) 
	{
		uint32_t code = static_cast<uint32_t>(event.GetCode());
		Aether::Log::Debug("KeyboardRelease {0}",code);
		return true;
	};
	

	dispatcher.Dispatch<Aether::KeyboardReleaseEvent,decltype(onKeyboardRelease)>(onKeyboardRelease);

	auto onKeyboardRepeat = [](Aether::KeyboardRepeatEvent& event)
	{
		uint32_t code = static_cast<uint32_t>(event.GetCode());
		Aether::Log::Debug("KeyboardRepeat {0}", code);
		return true;
	};


	dispatcher.Dispatch<Aether::KeyboardRepeatEvent>(onKeyboardRepeat);


	auto onMouseButtonRelease = [](Aether::MouseButtonPressEvent& event)
	{
		int button =(int) event.GetCode();
		Aether::Log::Debug("MouseButtonPress {0}", button);
		return true;
	};
	dispatcher.Dispatch<Aether::MouseButtonPressEvent>(onMouseButtonRelease);
	auto onMouseScroll = [](Aether::MouseScrollEvent& event)
	{
		float yoffset = event.GetYOffset();
		Aether::Log::Debug("MouseScroll yoffset {0}", yoffset);
		return true;
	};
	dispatcher.Dispatch<Aether::MouseScrollEvent>(onMouseScroll);
	
}


