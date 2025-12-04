#pragma once
#include <Core/Core.h>
#include <Render/Config.h>
namespace Aether::RenderGraph
{
struct VirtualResourceBase;
enum class TaskType : uint8_t
{
    RenderTask,
    ComputeTask,

    ImageLayoutTransitionTask,

    // transfer
    UploadBufferTask,
    UploadTextureTask,
    DownloadBufferTask,
    DownloadTextureTask,
};
struct TaskBase
{
    virtual ~TaskBase() = default;
    TaskBase(TaskType type) : type(type)
    {
    }
    TaskType type;
    std::vector<Borrow<VirtualResourceBase>> reads;
    std::vector<Borrow<VirtualResourceBase>> writes;
    std::vector<Borrow<VirtualResourceBase>> creates;
    size_t refCount = 0; // for culling when compile
    bool cullImmune = false;
    std::string tag;
};

} // namespace Aether::RenderGraph