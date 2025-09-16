#include "Id.h"
namespace Aether::Render
{
    HandleAllocator& HandleAllocator::GetSingleton()
    {
        static HandleAllocator instance;
        return instance;
    }
}