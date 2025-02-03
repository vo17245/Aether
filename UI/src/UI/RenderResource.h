#pragma once
#include "DynamicStagingBuffer.h"
#include "Render/RenderApi.h"
namespace Aether::UI
{
struct RenderResource
{
    Ref<DeviceDescriptorPool> m_DescriptorPool;
    Ref<DynamicStagingBuffer> m_StagingBuffer;
};
}