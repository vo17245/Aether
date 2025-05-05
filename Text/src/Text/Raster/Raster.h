#pragma once
#include "Render/RenderApi/DeviceSampler.h"
#include "Text/Font.h"
#include "QuadArrayMesh.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DevicePipeline.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include <Render/RenderApi.h>
#include <Render/Scene/Camera2D.h>
namespace Aether::Text
{
class Raster
{
public:
    struct RenderPassParam
    {
        // render resource
        DeviceCommandBufferView commandBuffer;
        DeviceRenderPassView renderPass;
        DeviceFrameBufferView frameBuffer;
        DeviceDescriptorPool& descriptorPool;
        // text
        Font& font;
        std::vector<uint32_t> glyphIndex;
        std::vector<Vec2f> glyphPosition;
        float scale=1.0;
        // storage
        uint32_t channel=0;// store mask in rgba(0 1 2 3)
    };
    struct RenderPassResource
    {
        DeviceBuffer uniformBuffer;
        DeviceMesh mesh;
    };

public:
    void Render(RenderPassParam& param,RenderPassResource& resource);
    bool Init(DeviceRenderPassView renderPass, bool enableBlend, DeviceDescriptorPool& descriptorPool);
    Camera2D& GetCamera()
    {
        return m_Camera;
    }
    RenderPassResource CreateRenderPassResource()
    {
        RenderPassResource resource;
        resource.uniformBuffer=DeviceBuffer::CreateForUniform(sizeof(HostUniformBuffer));
        return resource;
    }

private:
    bool CreateShader();
    bool CreatePipeline(DeviceRenderPassView renderPass, bool enableBlend);
    bool CreateDescriptorSet(DeviceDescriptorPool& descriptorPool); // per draw
    bool CreateCamera(const Vec2f& screenSize);
    void UpdateMesh(RenderPassParam& param,RenderPassResource& resource);
private:
    struct HostUniformBuffer
    {
        float mvp[16];
        float screenSize[2];
        float align[2];
    };
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceShader m_Shader;
    Camera2D m_Camera;
    DeviceBuffer m_StaggingBuffer;
    DeviceDescriptorSet m_DescriptorSet;
};
} // namespace Aether::Text