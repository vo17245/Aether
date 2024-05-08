#include "Layers/EditorLayer.h"
#include "Editor.h"
using namespace Aether;
using namespace Editor;
extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
};
int main()
{
    Application::Init();
    ::Aether::Editor::Editor* editor = new ::Aether::Editor::Editor();
    editor->Run();
    int ret = Application::Get().Run();
    delete editor;
    Application::Release();
    return ret;
}