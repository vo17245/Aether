#pragma once
#include "vma/vk_mem_alloc.h"
namespace Aether {
class Application;
namespace vk {

class Allocator
{
public:
    static VmaAllocator Get()
    {
        return GetSingleton().m_Allocator;
    }
    static void Init()
    {
        s_Instance = new Allocator();
    }
    static void Release()
    {
        delete s_Instance;
    }

private:
    ~Allocator()
    {
        vmaDestroyAllocator(m_Allocator);
    }

    static Allocator& GetSingleton()
    {
        return *s_Instance;
    }
    Allocator();

    VmaAllocator m_Allocator;
    static Allocator* s_Instance;
};
} // namespace vk
} // namespace Aether