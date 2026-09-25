#pragma once

#include <ProjectAsset/Types.h>

#include <memory>

namespace Aether::ProjectAssets
{
class ProjectWriteLock
{
public:
    ProjectWriteLock(const ProjectWriteLock&) = delete;
    ProjectWriteLock& operator=(const ProjectWriteLock&) = delete;
    ProjectWriteLock(ProjectWriteLock&&) noexcept;
    ProjectWriteLock& operator=(ProjectWriteLock&&) noexcept;
    ~ProjectWriteLock();

    static Result<std::unique_ptr<ProjectWriteLock>> Acquire(const std::filesystem::path& projectRoot);
    const std::filesystem::path& LockPath() const noexcept;
private:
    struct State;
    explicit ProjectWriteLock(std::unique_ptr<State> state);
    std::unique_ptr<State> m_State;
};
}
