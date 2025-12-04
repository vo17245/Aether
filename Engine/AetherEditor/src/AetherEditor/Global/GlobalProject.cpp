#include "GlobalProject.h"

namespace AetherEditor
{
    Aether::Scope<Aether::Project::Project> GlobalProject::s_Project;
    Aether::Delegate<void(Aether::Project::Project&)> GlobalProject::ProjectChangedEventHandler;
}