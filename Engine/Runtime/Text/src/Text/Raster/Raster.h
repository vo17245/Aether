#pragma once
#include "Text/Font/Font.h"
#include "QuadArrayMesh.h"
#include <Render/RHI.h>
#include <Render/Scene/Camera2D.h>
#include <Render/Mesh/GpuMesh.h>
#include <Render/RenderGraph/RenderGraph.h>
namespace Aether::Text
{
class Raster
{
public:
    struct RenderPassParam
    {
        // render resource
        RenderGraph::RenderGraph* renderGraph = nullptr;
        RenderGraph::RenderPassDesc renderPassDesc;
        // text
        Font& font;
        std::vector<uint32_t>& bufferGlyphInfoIndexes;//glyph indexes in font
        std::vector<Vec2f>& glyphPosition;
        float worldSize = 32.0f;
        Camera2D& camera;//@note 传入的camera需要提前调用过CalculateMatrix, raster中不会调用
        float z=0.0f;// z value in screen space
        Vec3f color = Vec3f(1.0f, 1.0f, 1.0f); 
    };
    struct RenderPassResource
    {
        rhi::UniformBuffer uniformBuffer;
        GpuMesh mesh;
    };

public:
    /**
     * @brief commandBuffer must in begin render pass state
     * */
    bool Render(RenderPassParam& param, RenderPassResource& resource);

    RenderPassResource CreateRenderPassResource()
    {
        RenderPassResource resource;
        resource.uniformBuffer = rhi::UniformBuffer::Create(sizeof(HostUniformBuffer));
        return resource;
    }
    enum class Keyword:uint32_t
    {
        // optional keywords
        ColorfulEdge=Bit(0),
        // optional keywords
        Oversampling=Bit(1),
        // require Fill or SDF
        Fill=Bit(3),
        Sdf=Bit(4),
    };
    using KeywordFlags = uint32_t;
    static std::optional<Raster> Create(bool enableBlend, 
                                        bool enableDepthTest,KeywordFlags keywords= PackFlags(Keyword::Fill))
    {
        Raster raster;
        raster.m_Keywords = keywords;
        bool res = raster.Init(enableBlend, enableDepthTest);
        if (!res)
        {
            return std::nullopt;
        }
        return raster;
    }
    Raster(Raster&&) = default;

private:
    bool Init(bool enableBlend, bool enableDepthTest);
    Raster() = default;

private:
    bool CreateShader();                                                            // on init
    bool CreatePipeline(bool enableBlend,bool enableDepthTest);                     // on init
    bool CreateDescriptorSet();                                                     // per draw
    bool UpdateMesh(RenderPassParam& param, RenderPassResource& resource);          // per draw
    bool UpdateUniformBuffer(RenderPassParam& param, RenderPassResource& resource); // per draw
    bool UpdateDescriptorSet(RenderPassResource& resource, RenderPassParam& param); // per draw
    bool RecordCommand(rhi::CommandList& commandBuffer, RenderPassResource& resource); // per graph task
private:
    struct HostUniformBuffer
    {
        float mvp[16];
        float color[4]={1.0,1.0,1.0,1.0}; // RGB ,A is not used,just for alignment
    };
    rhi::Pipeline m_Pipeline;
    rhi::StagingBuffer m_StaggingBuffer;
    rhi::DescriptorSet m_DescriptorSet;
    HostUniformBuffer m_HostUniformBuffer;
    rhi::Sampler m_GlyphTextureSampler;
    rhi::Sampler m_CurveTextureSampler;
    rhi::VertexShader m_VertexShader;
    rhi::PixelShader m_PixelShader;

private:
    // The glyph quads are expanded by this amount to enable proper
    // anti-aliasing. Value is relative to emSize.
    float m_Dilation = 0;
    float m_WorldSize = 0;
private:
    KeywordFlags m_Keywords = 0;
};
} // namespace Aether::Text