#include "GammaFilter.h"
#include "Render/Config.h"
#include "Render/RenderApi/DevicePipeline.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/RenderApi/DeviceShader.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Utils.h"
#include "Render/Vulkan/DescriptorPool.h"
#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderPass.h"
#include "vulkan/vulkan_core.h"
#include <ranges>
namespace Aether::WindowInternal
{
bool GammaFilter::CreatePipeline(const vk::RenderPass& renderPass)
{
    assert(Render::Api::Vulkan == Render::Config::RenderApi && "only support vulkan now");
    auto& shader = m_Shader.GetVk();
    auto& vert = shader.vertex;
    auto& frag = shader.fragment;
    auto& descriptorResource = m_DescriptorSet.GetVk();
    auto vertexLayout = m_Mesh.CreateVertexBufferLayouts();
    // create pipeline layout

    vk::PipelineLayout::Builder layoutBuilder;
    for (auto& layout : descriptorResource.layouts)
    {
        layoutBuilder.AddDescriptorSetLayout(layout);
    }
    auto pipelineLayout = layoutBuilder.Build();
    if (!pipelineLayout)
    {
        return false;
    }
    // create pipeline

    vk::GraphicsPipeline::Builder pipelineBuilder(renderPass, *pipelineLayout);
    pipelineBuilder.BeginColorAttachment();
    pipelineBuilder.EndColorAttachment();
    for (auto& layout : vertexLayout)
    {
        pipelineBuilder.PushVertexBufferLayouts(layout);
    }
    pipelineBuilder.AddVertexStage(vert, vert.GetEntryPoint().c_str());
    pipelineBuilder.AddFragmentStage(frag, frag.GetEntryPoint().c_str());
    auto pipeline = pipelineBuilder.Build();
    if (!pipeline)
    {
        return false;
    }
    m_Pipeline = std::move(pipeline.value());
    m_PipelineLayout = std::move(pipelineLayout.value());
    return true;
}
bool GammaFilter::CreateDescriptor(DeviceDescriptorPool& pool)
{
    auto set = pool.CreateSet(1, 0, 1);
    if (!set)
    {
        return false;
    }
    m_DescriptorSet = std::move(set);
    return true;
}
bool GammaFilter::Render(DeviceTexture& _from, DeviceCommandBufferView _commandBuffer, DeviceDescriptorPool& pool)
{
    auto& from = _from.GetVk();
    auto& commandBuffer = _commandBuffer.GetVk();
    auto& descriptorResource = m_DescriptorSet.GetVk();
    auto& pipeline = m_Pipeline.GetVk();
    auto& mesh = m_DeviceMesh.GetVk();
    CreateDescriptor(pool);
    // update uniform buffer
    UpdateDescriptor(_from);
    _commandBuffer.GetVk();
    // bind pipeline
    commandBuffer.BindPipeline(pipeline);
    // update descriptor set
    for (auto&& [index, set] : std::ranges::views::enumerate(descriptorResource.sets))
    {
        commandBuffer.BindDescriptorSet(set, m_PipelineLayout.GetVk(), index);
    }
    // draw mesh
    Render::Utils::VkDrawMesh(commandBuffer, mesh);
    return true;
}
bool GammaFilter::UpdateDescriptor(DeviceTexture& from)
{
    auto& descriptorResource = m_DescriptorSet.GetVk();
    // update texture sampler
    {
        auto& accessor = descriptorResource.samplers[0];
        auto& set = descriptorResource.sets[accessor.set];
        auto binding = accessor.binding;
        vk::DescriptorSetOperator op(set);
        op.BindSampler(binding, m_Sampler.GetVk(), from.GetOrCreateDefaultImageView().GetVk());
        op.Apply();
    }
    // update ubo
    {
        auto& accessor = descriptorResource.ubos[0];
        auto& set = descriptorResource.sets[accessor.set];
        auto binding = accessor.binding;

        vk::DescriptorSetOperator op(set);
        op.BindUBO(binding, m_UniformBuffer[m_FrameIndex].GetVk());
        op.Apply();
    }
    return true;
}
bool GammaFilter::CreateSampler()
{
    assert(Render::Api::Vulkan == Render::Config::RenderApi && "only support vulkan now");
    vk::Sampler::Builder builder;
    builder.SetMagFilter(vk::Sampler::Filter::Linear);
    builder.SetMinFilter(vk::Sampler::Filter::Linear);
    builder.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    builder.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
    builder.SetAnisotropyEnable(true);
    builder.SetMaxAnisotropy(16);
    auto sampler = builder.Build();
    if (!sampler)
    {
        return false;
    }
    m_Sampler = std::move(sampler.value());
    return true;
}
bool GammaFilter::CreateUbo()
{
    for (size_t i = 0; i < Render::Config::MaxFramesInFlight; ++i)
    {
        auto buffer = DeviceBuffer::CreateForUniform(sizeof(float));
        if (!buffer)
        {
            return false;
        }
        m_UniformBuffer[i] = std::move(buffer);
    }
    return true;
}
void GammaFilter::OnUpdate(PendingUploadList& uploadList)
{
    if (m_UboDirty > 0)
    {
        uploadList.UploadInFlightBuffer(std::span<uint8_t>((uint8_t*)&m_Gamma, (uint8_t*)&m_Gamma + sizeof(float)),
                                        m_UniformBuffer[m_FrameIndex], 0);
        --m_UboDirty;
    }
}
bool GammaFilter::CreateShader()
{
    static const char* frag = R"(
#version 450
layout(location=0) in vec2 v_TexCoord;
layout(location=0) out vec4 FragColor;

layout(std140,set=0,binding=0)uniform UniformBufferObject
{
    float gamma;
}ubo;
layout(set=1,binding=0) uniform sampler2D u_Texture;

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
std::expected<GammaFilter, std::string> GammaFilter::Create(const vk::RenderPass& renderPass,
                                                            DeviceDescriptorPool& pool)
{
    GammaFilter filter;
    filter.m_UboDirty = Render::Config::MaxFramesInFlight;
    if(!filter.CreateUbo())
    {
        return std::unexpected<std::string>("CreateUbo failed");
    }
    if (!filter.CreateSampler())
    {
        return std::unexpected<std::string>("CreateSampler failed");
    }
    if (!filter.CreateMesh())
    {
        return std::unexpected<std::string>("CreateMesh failed");
    }
    if (!filter.CreateShader())
    {
        return std::unexpected<std::string>("CreateShader failed");
    }
    if (!filter.CreateDescriptor(pool))
    {
        return std::unexpected<std::string>("CreateDescriptor failed");
    }
    if (!filter.CreatePipeline(renderPass))
    {
        return std::unexpected<std::string>("CreatePipeline failed");
    }

    return filter;
}
} // namespace Aether::WindowInternal