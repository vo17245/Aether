#include "Sandbox.h"
#include "TestLayer.h"
namespace Aether
{
	Sandbox::Sandbox()
	{
		auto layer=CreateRef<TestLayer>();
		PushLayer(layer);

	}
}


