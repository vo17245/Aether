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
        if (s_Instance) return;
        s_Instance = new Allocator();
    }
    static void Release()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
    static bool IsInitialized() { return s_Instance != nullptr; }

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
