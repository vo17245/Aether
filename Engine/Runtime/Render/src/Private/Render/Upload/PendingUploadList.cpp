#include "Render/Upload/PendingUploadList.h"
namespace Aether
{
void PendingUploadList::RecordCommand(rhi::CommandList& commandBuffer)
{
    for (auto& upload : m_UploadBufferList)
    {
        std::visit(
            [&](auto&& dstBuffer) {
                using T = std::decay_t<decltype(dstBuffer)>;
                if constexpr (std::is_same_v<T, rhi::VertexBuffer*>)
                {
                    commandBuffer.UploadVertexBuffer(upload.source->m_Buffer, *dstBuffer, upload.size,
                                                     upload.sourceOffset, upload.destinationOffset);
                }
                else if constexpr (std::is_same_v<T, rhi::IndexBuffer*>)
                {
                    commandBuffer.UploadIndexBuffer(upload.source->m_Buffer, *dstBuffer, upload.size,
                                                    upload.sourceOffset, upload.destinationOffset);
                }
            },
            upload.destination);
    }
    m_UploadBufferList.clear();
}
void PendingUploadList::OnUpdate(bool minilized)
{
    if (minilized)
    {
        return;
    }
    for (auto iter = m_StagingBuffers.begin(); iter != m_StagingBuffers.end();)
    {
        auto& buffer = *iter;
        buffer->m_TTL--;
        if (buffer->m_TTL <= 0)
        {
            iter = m_StagingBuffers.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

} // namespace Aether