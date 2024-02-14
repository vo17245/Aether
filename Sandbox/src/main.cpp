#include "Sandbox.h"
#include "Aether/Core/Application.h"
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
};
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