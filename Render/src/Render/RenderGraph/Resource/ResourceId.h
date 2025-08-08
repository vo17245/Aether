#pragma once
#include "../Handle.h"
#include <Core/Core.h>
namespace Aether::RenderGraph
{
// typed resource id
template <typename T>
struct ResourceId
{
    Handle handle;
    static constexpr ResourceId<T> CreateInvalid()
    {
        return ResourceId<T>{Handle{Handle::InvalidId,0}};
    }
};
class ResourceIdAllocator
{
public:
    template <typename T>
    ResourceId<T> Allocate()
    {
        Handle handle = m_Allocator.Allocate();
        return ResourceId<T>{handle};
    }
    template <typename T>
    Handle::Version GetNextVersion(ResourceId<T> id)
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
struct Hash<RenderGraph::ResourceId<T>>
{
    size_t operator()(const RenderGraph::ResourceId<T>& id) const
    {
        return Hash<RenderGraph::Handle>()(id.handle);
    }
};
} // namespace Aether