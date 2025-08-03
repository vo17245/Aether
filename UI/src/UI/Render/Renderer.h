#pragma once
#include "Render/RenderApi.h"
#include "Quad.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "QuadArrayMesh.h"
#include "Render/RenderApi/DeviceSampler.h"
#include "Render/Temporary.h"
#include "RenderResource.h"
#include "UI/Base/Pool.h"
#include "Render/Scene/Camera2D.h"
namespace Aether::UI
{
/**
 * TODO 
 * allow multiple draw in one frame
 * use only one sampler, instead of create sampler for each draw
*/
class Renderer
{
public:
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = default;
    void Begin( DeviceFrameBufferView frameBuffer, const Camera2D& camera);
    void DrawQuad(const Quad& quad);
    void End(DeviceCommandBufferView _commandBuffer);
    void Reset();
    void Clear();
    /**
     * @brief use renderPass to init pipeline in vulkan backend
     *       in vulkan The pipeline must only be used with a render pass instance compatible with the one provided.
     *       See Render Pass Compatibility for more information.
     */
    static std::expected<Renderer, std::string> Create(DeviceRenderPassView renderPass, const RenderResource& resource);
    // for unit test
    static Renderer CreateEmpty();
   
    inline void OnFrameBegin()
    {
        #ifdef AETHER_RUNTIME_CHECK
        m_IsBusy=false;
        #endif
    }

private:
    Renderer() = default;
    struct SamplerCreator
    {
        DeviceSampler operator()()
        {
            return CreateQuadWithTextureSampler();
        }
    };
    using DeviceSamplerPool = Pool<DeviceSampler, SamplerCreator>;
    DeviceSamplerPool m_SamplerPool;
    RenderResource m_RenderResource;
    // ====== basic
    struct BasicUniformBlock
    {
        Mat4f model = Mat4f::Identity();
        Mat4f view = Mat4f::Identity();
    };
    static_assert(sizeof(BasicUniformBlock) == 4 * 16 * 2);
    DeviceBuffer m_BasicUboBuffer;
    DevicePipeline m_BasicPipeline; // only draw quad with color
    DevicePipelineLayout m_BasicPipelineLayout;
    DeviceDescriptorSet m_BasicDescriptorSet;
    DeviceShader m_BasicShader;
    BasicUniformBlock m_BasicUboLocalBuffer;
    bool CreateBasicPipeline(DeviceRenderPassView _renderPass);
    bool CreateBasicDescriptorSet(); // per frame create
    bool CreateBasicShader();
    bool CreateBasicBuffer();
    bool UploadBasicBuffer(); // per frame update

    //============
    //============ quad with texture
    struct QuadWithTexture
    {
        Ref<DeviceTexture> texture;
        QuadArrayMesh mesh;
        DeviceDescriptorSet descriptorSet;
        DeviceSamplerPool::Resource sampler;
    };
    std::vector<QuadWithTexture> m_QuadsWithTexture;
    struct QuadWithTextureUniformBlock
    {
        Mat4f model = Mat4f::Identity();
        Mat4f view = Mat4f::Identity();
        Mat4f texCoord = Mat4f::Identity(); // 只有左上角的3x3矩阵有用，其他部分是为了shader中16字节内存对齐
    };
    DeviceBuffer m_QuadWithTextureUboBuffer;
    DevicePipeline m_QuadWithTexturePipeline;
    DevicePipelineLayout m_QuadWithTexturePipelineLayout;
    DeviceShader m_QuadWithTextureShader;
    QuadWithTextureUniformBlock m_QuadWithTextureUboLocalBuffer;
    bool CreateQuadWithTexturePipeline(DeviceRenderPassView _renderPass);
    DeviceSampler m_QuadWithTextureSampler;
    bool CreateQuadWithTextureShader();
    DeviceDescriptorSet CreateQuadWithTextureDescriptorSet();
    static DeviceSampler CreateQuadWithTextureSampler();
    bool UpdateQuadWithTextureDeviceUniformBuffer();
    bool UpdateQuadWithTextureDescriptor(QuadWithTexture& quad);
    bool CreateQuadWithTextureBuffer();
    //============
#ifdef AETHER_RUNTIME_CHECK
    bool m_IsBusy=false;
#endif
private:
    QuadArrayMesh m_BasicQuadArray; // quad with color
private:
    DeviceFrameBufferView m_FrameBuffer;

private:
    DeviceCommandBuffer m_TransformCommandBuffer; // for vertex data transform
private:
    Temporary<DeviceMesh> m_Temporary;
    Camera2D m_Camera;
};
} // namespace Aether::UI