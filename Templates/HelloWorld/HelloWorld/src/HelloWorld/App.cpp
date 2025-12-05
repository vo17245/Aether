#include <Entry/Application.h>
#include <Debug/Log.h>
using namespace Aether;
class HelloWorld : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        LogD("HelloWorld");
    }
    virtual const char* GetName() const override
    {
        return "HelloWorld";
    }
    virtual WindowCreateParam MainWindowCreateParam() override
    {
        auto param = WindowCreateParam{};
        param.title = "HelloWorld";
        param.imGuiEnableClear = false;
        return param;
    }
};
DEFINE_APPLICATION(HelloWorld);