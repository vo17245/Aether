#pragma once
#include "TaskBase.h"
#include "Resource/AccessId.h"
#include <Render/RenderApi.h>
namespace Aether::RenderGraph
{
struct UploadBufferTask : public TaskBase
{
    UploadBufferTask() : TaskBase(TaskType::UploadBufferTask) {}
    AccessId<DeviceBuffer> source;
    AccessId<DeviceBuffer> destination;
};
struct UploadTextureTask : public TaskBase
{
    UploadTextureTask() : TaskBase(TaskType::UploadTextureTask) {}
    AccessId<DeviceBuffer> source;
    AccessId<DeviceTexture> destination;
    
};
struct DownloadBufferTask : public TaskBase
{
    DownloadBufferTask() : TaskBase(TaskType::DownloadBufferTask) {}
    AccessId<DeviceBuffer> source;
    AccessId<DeviceBuffer> destination;
};
struct DownloadTextureTask : public TaskBase
{
    DownloadTextureTask() : TaskBase(TaskType::DownloadTextureTask) {}
    AccessId<DeviceTexture> source;
    AccessId<DeviceBuffer> destination;
};

} // namespace Aether::RenderGraph