#pragma once
#include "Resource.h"
namespace Aether::TaskGraph
{
    struct ResourceSlot
    {
        std::array<Scope<ResourceBase>,Render::Config::InFlightFrameResourceSlots> resources;
        size_t resourceCount=0;// 0,1 or Render::Config::MaxFramesInFlight
    };
    class ResourcePool
    {
    public:

    private:
        std::vector<ResourceSlot> m_Slots;
    };
}