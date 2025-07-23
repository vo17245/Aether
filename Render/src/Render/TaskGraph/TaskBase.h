#pragma once
#include <vector>
#include <functional>
#include <Render/RenderApi.h>
namespace Aether::TaskGraph
{
class ResourceBase;
class TaskBase
{
public:
    virtual ~TaskBase() = default;
    std::vector<ResourceBase*> creates;
    std::vector<ResourceBase*> reads;
    std::vector<ResourceBase*> writes;
    size_t refCount = 0; // for task graph compilation
    /**
     * @note
     * if false, task will be culled if not affect retained resources
     * if true, task will not be culled even if it does not affect retained resources
    */ 
    bool cullImmune = false; 
};

} // namespace Aether::TaskGraph