#include "World.h"
#include "System.h"

#include <algorithm>
#include <format>
#include <functional>
#include <exception>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace Aether
{

World::World() = default;
World::~World() = default;

void World::OnUpdate(float deltaTime)
{
    Dispatch([&](System& system) { system.OnUpdate(deltaTime); });
}
void World::OnUpdatePhase(SystemUpdatePhase phase, float deltaTime)
{
    BuildPhaseExecutionOrder();
    ++m_DispatchDepth;
    try
    {
        for (auto* system : m_ExecutionOrder)
        {
            if (system->GetUpdatePhase() == phase)
                system->OnUpdatePhase(phase, deltaTime);
        }
    }
    catch (...)
    {
        --m_DispatchDepth;
        throw;
    }
    --m_DispatchDepth;
}
bool World::NeedRebuildRenderGraph()
{
    EnsureExecutionOrder();
    ++m_DispatchDepth;
    try
    {
        for (auto* system : m_ExecutionOrder)
        {
            if (system->NeedRebuildRenderGraph())
            {
                --m_DispatchDepth;
                return true;
            }
        }
    }
    catch (...)
    {
        --m_DispatchDepth;
        throw;
    }
    --m_DispatchDepth;
    return false;
}
void World::OnUpload(PendingUploadList& uploadList)
{
    Dispatch([&](System& system) { system.OnUpload(uploadList); });
}
void World::OnEvent(Event& event)
{
    Dispatch([&](System& system) { system.OnEvent(event); });
}
void World::PushSystem(Scope<System>&& system)
{
    if (m_DispatchDepth != 0)
        throw std::logic_error("cannot add a World system while dispatching a callback");
    if (!system)
        throw std::invalid_argument("World::PushSystem cannot attach a null system");

    auto* rawSystem = system.get();
    const bool previousOrderDirty = m_OrderDirty;
    m_Systems.push_back(std::move(system));
    m_OrderDirty = true;
    m_PhaseOrderValid = false;
    try
    {
        rawSystem->OnAttach(this);
    }
    catch (...)
    {
        try
        {
            rawSystem->OnDetach();
        }
        catch (...)
        {
        }
        m_Systems.pop_back();
        m_OrderDirty = previousOrderDirty;
        m_PhaseOrderValid = false;
        throw;
    }
}
void World::EraseSystem(System* system)
{
    if (m_DispatchDepth != 0)
        throw std::logic_error("cannot remove a World system while dispatching a callback");
    auto iter = std::find_if(m_Systems.begin(), m_Systems.end(), [&](const Scope<System>& ptr) { return ptr.get() == system; });
    if (iter != m_Systems.end())
    {
        std::exception_ptr detachFailure;
        try { (*iter)->OnDetach(); }
        catch (...) { detachFailure = std::current_exception(); }
        m_Systems.erase(iter);
        m_OrderDirty = true;
        m_PhaseOrderValid = false;
        if (detachFailure) std::rethrow_exception(detachFailure);
    }
}
void World::OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
{
    Dispatch([&](System& system) { system.OnBuildRenderGraph(renderGraph); });
}
void World::OnFrameBegin(std::uint32_t frameSlot)
{
    Dispatch([&](System& system) { system.OnFrameBegin(frameSlot); });
}
void World::ExtractRenderData(Render::RenderFeatureFrame& frame)
{
    Dispatch([&](System& system) { system.ExtractRenderData(frame); });
}

void World::BuildExecutionOrder()
{
    if (m_DispatchDepth != 0)
        throw std::logic_error("cannot rebuild World system order while dispatching a callback");

    const auto count = m_Systems.size();
    std::vector<std::string> signatures;
    signatures.reserve(count);
    std::unordered_map<std::string, std::size_t> indices;
    indices.reserve(count);

    for (std::size_t index = 0; index < count; ++index)
    {
        const auto signatureView = m_Systems[index]->GetSignature();
        if (signatureView.empty())
            throw std::logic_error("World system signature cannot be empty");

        std::string signature(signatureView);
        if (!indices.emplace(signature, index).second)
            throw std::logic_error(std::format("duplicate World system signature '{}'", signature));
        signatures.push_back(std::move(signature));
    }

    std::vector<std::vector<std::size_t>> dependents(count);
    std::vector<std::size_t> indegrees(count, 0);
    for (std::size_t dependentIndex = 0; dependentIndex < count; ++dependentIndex)
    {
        std::unordered_set<std::string> seenDependencies;
        for (const auto dependencyView : m_Systems[dependentIndex]->GetDependencies())
        {
            const std::string dependency(dependencyView);
            if (!seenDependencies.emplace(dependency).second)
            {
                throw std::logic_error(std::format("system '{}' declares duplicate dependency '{}'",
                                                    signatures[dependentIndex], dependency));
            }

            const auto dependencyIter = indices.find(dependency);
            if (dependencyIter == indices.end())
            {
                throw std::logic_error(std::format("system '{}' depends on missing system '{}'",
                                                    signatures[dependentIndex], dependency));
            }
            if (dependencyIter->second == dependentIndex)
            {
                throw std::logic_error(std::format("system '{}' cannot depend on itself", signatures[dependentIndex]));
            }

            dependents[dependencyIter->second].push_back(dependentIndex);
            ++indegrees[dependentIndex];
        }
    }

    std::priority_queue<std::size_t, std::vector<std::size_t>, std::greater<>> ready;
    for (std::size_t index = 0; index < count; ++index)
    {
        if (indegrees[index] == 0)
            ready.push(index);
    }

    std::vector<System*> executionOrder;
    executionOrder.reserve(count);
    while (!ready.empty())
    {
        const auto index = ready.top();
        ready.pop();
        executionOrder.push_back(m_Systems[index].get());

        for (const auto dependentIndex : dependents[index])
        {
            if (--indegrees[dependentIndex] == 0)
                ready.push(dependentIndex);
        }
    }

    if (executionOrder.size() != count)
    {
        const auto cycleIter = std::find_if(indegrees.begin(), indegrees.end(), [](std::size_t degree) {
            return degree != 0;
        });
        const auto cycleIndex = static_cast<std::size_t>(std::distance(indegrees.begin(), cycleIter));
        throw std::logic_error(std::format("dependency cycle includes system '{}'", signatures[cycleIndex]));
    }

    m_ExecutionOrder = std::move(executionOrder);
    m_OrderDirty = false;
    m_PhaseOrderValid = false;
}

void World::BuildPhaseExecutionOrder()
{
    if (m_PhaseOrderValid && !m_OrderDirty)
        return;
    EnsureExecutionOrder();

    std::unordered_map<std::string, SystemUpdatePhase> phases;
    phases.reserve(m_Systems.size());
    for (const auto& system : m_Systems)
        phases.emplace(std::string(system->GetSignature()), system->GetUpdatePhase());

    for (const auto& system : m_Systems)
    {
        const auto dependentPhase = system->GetUpdatePhase();
        for (const auto dependency : system->GetDependencies())
        {
            const auto dependencyPhase = phases.at(std::string(dependency));
            if (SystemUpdatePhaseOrder(dependencyPhase) > SystemUpdatePhaseOrder(dependentPhase))
                throw std::logic_error(std::format("system '{}' depends on later update phase system '{}'",
                                                    system->GetSignature(), dependency));
        }
    }
    m_PhaseOrderValid = true;
}

void World::ReplaceDataFrom(World& detached)
{
    if (m_DispatchDepth != 0 || detached.m_DispatchDepth != 0)
        throw std::logic_error("cannot replace World data while either World is dispatching callbacks");
    if (!detached.m_Systems.empty())
        throw std::logic_error("replacement World must not own Systems");
    m_Registry.swap(detached.m_Registry);
    for (auto& system : m_Systems)
        system->OnWorldDataReplaced();
}

std::vector<std::string_view> World::ExecutionOrderSignatures()
{
    EnsureExecutionOrder();
    std::vector<std::string_view> signatures;
    signatures.reserve(m_ExecutionOrder.size());
    for (const auto* system : m_ExecutionOrder)
        signatures.push_back(system->GetSignature());
    return signatures;
}

void World::EnsureExecutionOrder()
{
    if (m_OrderDirty)
        BuildExecutionOrder();
}

template <typename Callback>
void World::Dispatch(Callback&& callback)
{
    EnsureExecutionOrder();
    ++m_DispatchDepth;
    try
    {
        for (auto* system : m_ExecutionOrder)
            callback(*system);
    }
    catch (...)
    {
        --m_DispatchDepth;
        throw;
    }
    --m_DispatchDepth;
}
} // namespace Aether
