#include <Window/CloseRequestGate.h>

#include <cassert>

int main()
{
    Aether::CloseRequestGate ordinaryWindow;
    ordinaryWindow.Request();
    assert(ordinaryWindow.ShouldClose());
    assert(!ordinaryWindow.RequestPending());

    Aether::CloseRequestGate gatedWindow;
    int requests = 0;
    gatedWindow.SetHandler([&] { ++requests; });
    gatedWindow.Request();
    gatedWindow.Request();
    assert(requests == 1);
    assert(gatedWindow.RequestPending());
    assert(!gatedWindow.ShouldClose());
    gatedWindow.Cancel();
    assert(!gatedWindow.RequestPending());
    assert(!gatedWindow.ShouldClose());
    gatedWindow.Request();
    assert(requests == 2);
    gatedWindow.Confirm();
    assert(!gatedWindow.RequestPending());
    assert(gatedWindow.ShouldClose());
}
