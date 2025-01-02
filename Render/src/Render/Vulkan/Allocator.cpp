#include "Allocator.h"

#include "GlobalRenderContext.h"
namespace Aether {
namespace vk {

Allocator* Allocator::s_Instance;
static VmaAllocator CreateVmaAllocator()
{
    VmaAllocator allocator;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = GRC::GetPhysicalDevice();
    allocatorInfo.device = GRC::GetDevice();
    allocatorInfo.instance = GRC::GetInstance();

    vmaCreateAllocator(&allocatorInfo, &allocator);
    return allocator;
}
Allocator::Allocator()
{
    m_Allocator = CreateVmaAllocator();
}
}
} // namespace Aether::vk