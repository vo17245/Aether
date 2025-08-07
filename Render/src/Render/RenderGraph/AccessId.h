#pragma once
#include "Handle.h"
#include <Core/Core.h>
namespace Aether::RenderGraph
{
// typed resource id
template <typename T>
struct AccessId
{
    Handle handle;
};
class AccessIdAllocator
{
public:
    template <typename T>
    AccessId<T> Allocate()
    {
        Handle handle = m_Allocator.Allocate();
        return AccessId<T>{handle};
    }
    template <typename T>
    Handle::Version GetNextVersion(AccessId<T> id)
    {
        return m_Allocator.GetNextVersion(id.handle.id);
    }

private:
    HandleAllocator m_Allocator;
};
} // namespace Aether::RenderGraph
namespace Aether
{
template <typename T>
struct Hash<RenderGraph::AccessId<T>>
{
    size_t operator()(const RenderGraph::AccessId<T>& id) const
    {
        return Hash<RenderGraph::Handle>()(id.handle);
    }
};
} // namespace Aether