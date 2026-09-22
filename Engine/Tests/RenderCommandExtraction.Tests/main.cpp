#include <Window/Layer.h>

#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}

int main()
try
{
    int accepted = 0;
    int cancelled = 0;
    int executed = 0;
    Aether::RenderCommandExtraction extraction;
    extraction.Add(Aether::Render::MakeRenderCommand([&] { ++executed; }, 37),
                   [&] { ++accepted; }, [&] { ++cancelled; });
    Check(!extraction.Empty(), "extraction did not own its command");
    auto commands = extraction.TakeCommands();
    Check(commands.size() == 1, "command transfer was not atomic");
    extraction.Accept();
    extraction.Accept();
    Check(accepted == 1 && cancelled == 0, "accept notification was not exactly once");
    Aether::Render::RenderFrameContext context;
    commands[0]->Execute(context);
    Check(executed == 1 && commands[0]->PayloadBytes() == 37, "owned command was corrupted");

    Aether::RenderCommandExtraction rejected;
    rejected.Add(Aether::Render::MakeRenderCommand([] {}), {}, [&] { ++cancelled; });
    rejected.Cancel();
    rejected.Cancel();
    Check(cancelled == 1 && rejected.Empty(), "cancel did not release the unaccepted command exactly once");
    std::cout << "Render command extraction tests passed\n";
    return 0;
}
catch (const std::exception& exception)
{
    std::cerr << exception.what() << '\n';
    return 1;
}
