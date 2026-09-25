#pragma once
#include "TaskBase.h"
#include "Resource/AccessId.h"
#include <Render/RHI.h>
#include <functional>
#include <span>
namespace Aether::RenderGraph
{
// texture
struct UploadTextureTask : public TaskBase
{
    UploadTextureTask() : TaskBase(TaskType::UploadTextureTask) {}
    AccessId<rhi::StagingBuffer> source;
    AccessId<rhi::Texture2D> destination;
    
};
using TextureDownloadRegion = rhi::TextureDownloadRegion;
using TextureDownloadRegions = std::function<std::span<const TextureDownloadRegion>()>;
struct DownloadTextureTask : public TaskBase
{
    DownloadTextureTask() : TaskBase(TaskType::DownloadTextureTask) {}
    AccessId<rhi::Texture2D> source;
    AccessId<rhi::StagingBuffer> destination;
    TextureDownloadRegions regionsForCurrentSlot;
    void Execute(rhi::CommandList& commandBuffer, ResourceAccessor& resourceAccessor)
    {
        auto* src = resourceAccessor.GetResource(source);
        auto* dst = resourceAccessor.GetResource(destination);
        if (!src || !dst || !regionsForCurrentSlot) return;
        commandBuffer.DownloadTexture(*src, *dst, regionsForCurrentSlot());
    }
};
// vertex buffer
struct UploadVertexBufferTask : public TaskBase
{
    UploadVertexBufferTask() : TaskBase(TaskType::UploadVertexBufferTask) {}
    AccessId<rhi::StagingBuffer> source;
    AccessId<rhi::VertexBuffer> destination;
};

struct DownloadVertexBufferTask : public TaskBase
{
    DownloadVertexBufferTask() : TaskBase(TaskType::DownloadVertexBufferTask) {}
    AccessId<rhi::VertexBuffer> source;
    AccessId<rhi::StagingBuffer> destination;
};
// index buffer
struct UploadIndexBufferTask : public TaskBase
{
    UploadIndexBufferTask() : TaskBase(TaskType::UploadIndexBufferTask) {}
    AccessId<rhi::StagingBuffer> source;
    AccessId<rhi::IndexBuffer> destination;
};
struct DownloadIndexBufferTask : public TaskBase
{
    DownloadIndexBufferTask() : TaskBase(TaskType::DownloadIndexBufferTask) {}
    AccessId<rhi::IndexBuffer> source;
    AccessId<rhi::StagingBuffer> destination;
};
// uniform buffer
struct UploadUniformBufferTask : public TaskBase
{
    UploadUniformBufferTask() : TaskBase(TaskType::UploadUniformBufferTask) {}
    AccessId<rhi::StagingBuffer> source;
    AccessId<rhi::UniformBuffer> destination;
};
struct DownloadUniformBufferTask : public TaskBase
{
    DownloadUniformBufferTask() : TaskBase(TaskType::DownloadUniformBufferTask) {}
    AccessId<rhi::UniformBuffer> source;
    AccessId<rhi::StagingBuffer> destination;
};
// RWStructuredBuffer
struct UploadRWStructuredBufferTask : public TaskBase
{
    UploadRWStructuredBufferTask() : TaskBase(TaskType::UploadRWStructuredBufferTask) {}
    AccessId<rhi::StagingBuffer> source;
    AccessId<rhi::RWStructuredBuffer> destination;
};
struct DownloadRWStructuredBufferTask : public TaskBase
{
    DownloadRWStructuredBufferTask() : TaskBase(TaskType::DownloadRWStructuredBufferTask) {}
    AccessId<rhi::RWStructuredBuffer> source;
    AccessId<rhi::StagingBuffer> destination;
};

} // namespace Aether::RenderGraph