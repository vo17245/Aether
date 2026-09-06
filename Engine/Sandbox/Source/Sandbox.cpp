#include <Entry/Application.h>

using namespace Aether;

class Sandbox : public Application
{
public:
    const char* GetName() const override
    {
        return "Sandbox";
    }
};

DEFINE_APPLICATION(Sandbox);

