#pragma once
#include <Project/Project.h>
namespace AetherEditor
{

class GlobalProject
{
public:
    static Aether::Project::Project* GetProject()
    {
        return s_Project.get();
    }
    static void SetProject(Aether::Scope<Aether::Project::Project>&& project)
    {
        s_Project = std::move(project);
        ProjectChangedEventHandler.Broadcast(*s_Project);
    }
private:
    static Aether::Scope<Aether::Project::Project> s_Project;
public:
    static Aether::Delegate<void(Aether::Project::Project&)> ProjectChangedEventHandler;
};
}
