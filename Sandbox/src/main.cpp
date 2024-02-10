#include "Sandbox.h"
#include "Aether/Core/Application.h"
using namespace Aether;
int main()
{
	//aether init
	Application::Init();
	//define app
	Sandbox sandbox;
	//aether release
	int rc=Application::Get().Run();
	Application::Release();
	return rc;
}