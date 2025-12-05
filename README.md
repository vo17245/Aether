## Quick Start

1. Compile Engine
```sh
cd Engine/Scripts
./CreateProjectDebug.bat
./Build.bat
./CreatePackagesDebug.bat
```
cmake packages packed in Aether/Engine/Packages
2. Compile Dependencies

```sh
git clone https://github.com/vo17245/AetherDependencies
cd AetherDependencies
./CreateProjectDebug.bat
./Build.bat
./CreatePackagesDebug.bat
```
cmake packages packed in AetherDependencies/Packages

3. Add Aether and AetherDependencies packages path to your cmake prefix path
   
4. cmake config in your project

```cmake
find_package(Entry REQUIRED CONFIG)
target_link_libraries(${MODULE_NAME} PUBLIC Aether::Entry)
```
5. hello word
Application.cpp
```cpp
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
```