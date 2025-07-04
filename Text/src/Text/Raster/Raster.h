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
        std::vector<uint32_t>& unicodes;
        std::vector<Vec2f>& glyphPosition;
        float worldSize = 32.0f;
        Camera2D& camera;//@note 传入的camera需要提前调用过CalculateMatrix, raster中不会调用
        float z=0.0f;// z value in screen space
        Vec3f color = Vec3f(1.0f, 1.0f, 1.0f); 
    };
    struct RenderPassResource
    {
        DeviceBuffer uniformBuffer;
        DeviceMesh mesh;
    };

public:
    /**
     * @brief commandBuffer must in begin render pass state
     * */
    bool Render(RenderPassParam& param, RenderPassResource& resource);

    RenderPassResource CreateRenderPassResource()
    {
        RenderPassResource resource;
        resource.uniformBuffer = DeviceBuffer::CreateForUniform(sizeof(HostUniformBuffer));
        return resource;
    }
    static std::optional<Raster> Create(DeviceRenderPassView renderPass, bool enableBlend, DeviceDescriptorPool& descriptorPool,
                                        bool enableDepthTest)
    {
        Raster raster;
        bool res = raster.Init(renderPass, enableBlend, descriptorPool,enableDepthTest);
        if (!res)
        {
            return std::nullopt;
        }
        return raster;
    }
    Raster(Raster&&) = default;

private:
    bool Init(DeviceRenderPassView renderPass, bool enableBlend, DeviceDescriptorPool& descriptorPool,bool enableDepthTest);
    Raster() = default;

private:
    bool CreateShader();                                                            // on init
    bool CreatePipeline(DeviceRenderPassView renderPass, bool enableBlend,bool enableDepthTest);         // on init
    bool CreateDescriptorSet(DeviceDescriptorPool& descriptorPool);                 // per draw
    bool UpdateMesh(RenderPassParam& param, RenderPassResource& resource);          // per draw
    bool UpdateUniformBuffer(RenderPassParam& param, RenderPassResource& resource); // per draw
    bool UpdateDescriptorSet(RenderPassResource& resource, RenderPassParam& param); // per draw
    bool RecordCommand(RenderPassParam& param, RenderPassResource& resource);       // per draw
private:
    struct HostUniformBuffer
    {
        float mvp[16];
        float color[4]={1.0,1.0,1.0,1.0}; // RGB ,A is not used,just for alignment
    };
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceShader m_Shader;
    DeviceBuffer m_StaggingBuffer;
    DeviceDescriptorSet m_DescriptorSet;
    HostUniformBuffer m_HostUniformBuffer;
    DeviceSampler m_GlyphTextureSampler;
    DeviceSampler m_CurveTextureSampler;

private:
    // The glyph quads are expanded by this amount to enable proper
    // anti-aliasing. Value is relative to emSize.
    float m_Dilation = 0;
    float m_WorldSize = 0;
};
} // namespace Aether::Text