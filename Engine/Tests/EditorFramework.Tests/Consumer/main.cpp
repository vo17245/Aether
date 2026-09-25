#include <EditorFramework/EditorHost.h>
#include <GameFeature/RuntimeFeatureRegistrar.h>

int main()
{
    Aether::EditorFramework::EditorHost host;
    host.Tick(1.0f / 60.0f);
    return host.GetWorld().IsValid(host.SessionEntity()) ? 0 : 1;
}
