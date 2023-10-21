#include "Aether/Core/Core.h"
#include <string>
namespace Test
{
	class EventTest :public Test
	{
	public:
		EventTest() {}
		~EventTest() {}
		//void OnRender() override;
		//void OnImGuiRender() override;
		void OnEvent(Aether::Event& event) override;
		//void OnUpdate(float sec) override;

	};
}