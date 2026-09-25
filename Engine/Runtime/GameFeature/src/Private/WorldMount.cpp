#include <GameFeature/WorldMount.h>

#include <algorithm>
#include <exception>

namespace Aether::GameFeatures
{
namespace
{
void RecordDiagnostic(std::vector<std::string>& diagnostics, std::string message) noexcept
{
    try { diagnostics.push_back(std::move(message)); }
    catch (...) {}
}
}

Serialization::Result<void> FeatureMountScope::Mount(World& world, const WorldMountContext& context,
                                                     const std::vector<SystemRegistration>& registrations)
{
    if (m_World) return std::unexpected(Serialization::ArchiveError{Serialization::ArchiveErrorCode::InvalidArgument, "mount scope is already active"});
    m_World = &world;
    try
    {
        m_Attached.reserve(registrations.size());
        for (const auto& registration : registrations)
        {
            if (std::find(registration.roles.begin(), registration.roles.end(), context.role) == registration.roles.end())
                continue;
            if (!registration.factory)
                throw std::invalid_argument("system registration has no factory: " + registration.signature);
            auto system = registration.factory(context);
            if (!system) throw std::invalid_argument("system factory returned null: " + registration.signature);
            if (system->GetSignature() != registration.signature || system->GetDependencies().size() != registration.dependencies.size()
                || system->GetUpdatePhase() != registration.phase)
                throw std::invalid_argument("system does not match registration: " + registration.signature);
            const auto actualDependencies = system->GetDependencies();
            for (std::size_t i = 0; i < actualDependencies.size(); ++i)
                if (actualDependencies[i] != registration.dependencies[i])
                    throw std::invalid_argument("system dependencies do not match registration: " + registration.signature);
            auto* raw = system.get();
            world.PushSystem(std::move(system));
            m_Attached.push_back(raw);
        }
        world.BuildPhaseExecutionOrder();
        return {};
    }
    catch (const std::exception& exception)
    {
        RecordDiagnostic(m_Diagnostics, exception.what());
    }
    catch (...)
    {
        RecordDiagnostic(m_Diagnostics, "unknown failure while mounting feature systems");
    }
    const std::string failure = m_Diagnostics.empty() ? "feature system mount failed" : m_Diagnostics.back();
    Rollback();
    return std::unexpected(Serialization::ArchiveError{Serialization::ArchiveErrorCode::CodecFailure, failure});
}

void FeatureMountScope::Rollback() noexcept
{
    if (!m_World) return;
    for (auto it = m_Attached.rbegin(); it != m_Attached.rend(); ++it)
    {
        try { m_World->EraseSystem(*it); }
        catch (const std::exception& exception) { RecordDiagnostic(m_Diagnostics, exception.what()); }
        catch (...) { RecordDiagnostic(m_Diagnostics, "system detach failed with an unknown exception"); }
    }
    m_Attached.clear();
    m_World = nullptr;
}

void FeatureMountScope::Shutdown() noexcept
{
    Rollback();
}
}
