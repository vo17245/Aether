#pragma once
#include <Render/RHI.h>
namespace Aether
{
    struct GpuMesh
    {
        std::vector<rhi::VertexBuffer> buffers;
        struct BufferBinding
        {
            uint32_t bufferIndex;
            uint32_t offset;
        };
        enum class IndexType
        {
            None,
            UINT16,
            UINT32,
        };
        std::vector<BufferBinding> vertexBufferBindings;
        IndexType indexType = IndexType::None;
        std::optional<rhi::IndexBuffer> indexBuffer;
    };
}