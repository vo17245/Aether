#pragma once
#include "DynamicStagingBuffer.h"
#include <Render/RHI.h>

namespace Aether::UI
{
struct RenderResource
{
    Ref<DynamicStagingBuffer> m_StagingBuffer;
};
} // namespace Aether::UI
