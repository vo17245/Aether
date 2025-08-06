#pragma once
#include "Resource.h"
namespace Aether::TaskGraph
{
class ResourceAccessor
{
public:
    ResourceAccessor(std::vector<Scope<ResourceBase>>& resources,
                     std::vector<std::array<size_t, Render::Config::InFlightFrameResourceSlots>>& resourceIdToIndex,
                     ResourceHandleAllocator& resourceHandleAllocator, uint32_t currentFrameIndex) :
        m_Resources(resources), m_ResourceIdToIndex(resourceIdToIndex),
        m_ResourceHandleAllocator(resourceHandleAllocator), m_CurrentFrameIndex(currentFrameIndex)
    {
    }
    template<typename ResourceType>
    ResourceType* GetResource(const ResourceId<ResourceType>& id)const
    {
        ResourceHandle::Version version=m_CurrentFrameIndex%m_ResourceHandleAllocator.GetCurrentVersion(id);
        return (ResourceType*)m_Resources[m_ResourceIdToIndex[id][version]].get();
    }
private:
    std::vector<Scope<ResourceBase>>& m_Resources;
    
    /**
     * @brief get resource index in m_Resources by m_ResourceIdToIndex[handle.id][handle.version]
     * get current frame resource version by m_CurrentFrameIndex%m_ResourceHandleAllocator.GetCurrentVersion(handle.id)
    */
    std::vector<std::array<size_t,Render::Config::InFlightFrameResourceSlots>>& m_ResourceIdToIndex;
    ResourceHandleAllocator& m_ResourceHandleAllocator;
    uint32_t m_CurrentFrameIndex = 0; // current frame index for in-flight resources
};
}