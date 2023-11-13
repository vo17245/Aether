#include "Sandbox.h"

int main()
{
	Aether::Sandbox sandbox;
	std::cout << (const char*)glGetString(GL_VERSION) << std::endl;
	return sandbox.Run();
}