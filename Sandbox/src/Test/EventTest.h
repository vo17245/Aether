#include "Aether/Core/Core.h"
#include "Test.h"
#include <string>
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Event/KeyboardEvent.h"
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