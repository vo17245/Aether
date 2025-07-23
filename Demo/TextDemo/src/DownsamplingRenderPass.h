#pragma once
#include <Render/RenderApi.h>
using namespace Aether;
class DownsamplingRenderPass
{
public:
    
    static std::optional<DownsamplingRenderPass> Create(DeviceDescriptorPool& pool);
    /**
     * @note
     * renderpass desc:
     * - color attachment: 1
     *   rgba8888     size:1/4        loadOp: clear  storeOp: store
    */
    bool Render(DeviceCommandBufferView& commandBuffer,DeviceTexture& texture,DeviceDescriptorPool& pool);
private:
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceDescriptorSet m_DescriptorSet;// create per draw
    DeviceSampler m_Sampler;//nearest
    DeviceMesh m_Mesh;
};