#pragma once
#include "TaskGraph.h"
namespace Aether
{
template <typename T>
concept RenderModule = requires(T t, TaskGraph::TaskGraph& g) {
    { t.RegisterTasks(g) } -> std::same_as<void>;
    { t.OnUpdate() } -> std::same_as<void>;
    { t.NeedRebuild() } -> std::same_as<bool>;
    { t.ClearRebuildFlag() } -> std::same_as<void>;
};
} // namespace Aether