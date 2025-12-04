#pragma once
#include "Render/RenderApi.h"
#include "Render/RenderApi/DeviceBuffer.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceMesh.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/RenderApi/DeviceShader.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Shader/ShaderSource.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Vulkan/DescriptorPool.h"
#include "Render/Vulkan/RenderPass.h"
#include "vulkan/vulkan_core.h"
#include <Render/Render.h>
namespace Aether::WindowInternal
{
class GammaFilter
{
public:
    
    static std::expected<GammaFilter,std::string> Create(const vk::RenderPass& renderPass,
    DeviceDescriptorPool& pool);
    bool Render(DeviceTexture& from,DeviceCommandBufferView commandBuffer,DeviceDescriptorPool& pool);
    void SetFrameIndex(uint32_t index)
    {
        m_FrameIndex=index;
    }
    void SetGamma(float gamma)
    {
        m_Gamma=gamma;
        m_UboDirty=Render::Config::MaxFramesInFlight;
    }
    void OnUpdate(PendingUploadList& uploadList);
private:
    float m_Gamma=1.0f;
    int m_UboDirty=0;
    uint32_t m_FrameIndex=0;
    bool CreateUbo();
    bool UpdateDescriptor(DeviceTexture& from);// per frame
    GammaFilter()=default;
    DeviceShader m_Shader;
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceDescriptorSet m_DescriptorSet;
    Mesh m_Mesh;
    DeviceMesh m_DeviceMesh;
    DeviceSampler m_Sampler;
    bool CreateSampler();
    DeviceBuffer m_UniformBuffer[Render::Config::InFlightFrameResourceSlots];

    bool CreateMesh()
    {
        m_Mesh=Geometry::CreateQuad();
        m_DeviceMesh=DeviceMesh::Create(m_Mesh);
        if(!m_DeviceMesh)
        {
            return false;
        }
        return true;
    }

    bool CreatePipeline(const vk::RenderPass& renderPass);
    bool CreateDescriptor(DeviceDescriptorPool& pool);// per frame
    bool CreateShader();
}; 
}