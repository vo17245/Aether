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
namespace Aether::UI
{
class GammaFilter
{
public:

    static std::expected<GammaFilter,std::string> Create(const DeviceRenderPassView& renderPass,
    DeviceDescriptorPool* descriptorPool)
    {
        GammaFilter filter;
        filter.m_DescriptorPool = descriptorPool;
        filter.m_UniformBuffer=DeviceBuffer::CreateForUniform(sizeof(UniformBuffer));
        if (!filter.CreateSampler())
        {
            return std::unexpected<std::string>("CreateSampler failed");
        }
        if(!filter.CreateMesh())
        {
            return std::unexpected<std::string>("CreateMesh failed");
        }
        if (!filter.CreateShader())
        {
            return std::unexpected<std::string>("CreateShader failed");
        }
        if (!filter.CreateDescriptor())
        {
            return std::unexpected<std::string>("CreateDescriptor failed");
        }
        if (!filter.CreatePipeline(renderPass))
        {
            return std::unexpected<std::string>("CreatePipeline failed");
        }
        
        return filter;
    }
    bool Render(DeviceTexture& from,DeviceFrameBufferView to,DeviceCommandBufferView commandBuffer);
    void SetGamma(float gamma)
    {
        m_HostUniformBuffer.gamma=gamma;
    }

private:
    bool UpdateDescriptor(DeviceTexture& from);// per frame
    GammaFilter()=default;
    DeviceShader m_Shader;
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceDescriptorPool* m_DescriptorPool = nullptr; // not owned
    DeviceDescriptorSet m_DescriptorSet;
    Mesh m_Mesh;
    DeviceBuffer m_UniformBuffer;
    DeviceMesh m_DeviceMesh;
    DeviceSampler m_Sampler;
    bool CreateSampler();

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
    struct UniformBuffer
    {
        float gamma=2.2f;
    } m_HostUniformBuffer;
    bool CreatePipeline(const DeviceRenderPassView renderPass);
    bool CreateDescriptor();// per frame
    bool CreateShader()
    {
        static const char* frag = R"(
#version 450
layout(location=0) in vec2 v_TexCoord;
layout(location=0) out vec4 FragColor;
layout(set=1,binding=0) uniform sampler2D u_Texture;
layout(set=0,binding=0) uniform UniformBuffer
{
    float gamma;
}ubo;
void main()
{
vec4 textureColor=texture(u_Texture,v_TexCoord);
FragColor=vec4(pow(textureColor.rgb,vec3(ubo.gamma)),textureColor.a);
}
)";
        static const char* vert = R"(
        #version 450
        layout(location=0) in vec2 a_Position;
        layout(location=1) in vec2 a_TexCoord;
        layout(location=0) out vec2 v_TexCoord;
        void main()
        {
            gl_Position=vec4(a_Position,0.0,1.0);
            v_TexCoord=a_TexCoord;
        }
        )";
        ShaderSource vertSrc(ShaderStageType::Vertex, ShaderLanguage::GLSL, vert);
        ShaderSource fragSrc(ShaderStageType::Fragment, ShaderLanguage::GLSL, frag);
        auto shader = DeviceShader::Create(vertSrc, fragSrc);
        if (!shader)
        {
            return false;
        }
        m_Shader = std::move(*shader);
        return true;
    }
};
} // namespace Aether::UI