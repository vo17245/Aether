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
    auto set = pool.CreateSet(0, 0, 1);
    if (!set)
    {
        return false;
    }
    m_DescriptorSet = std::move(set);
    return true;
}
bool GammaFilter::Render(DeviceTexture& _from, DeviceCommandBufferView _commandBuffer,DeviceDescriptorPool& pool)
{
    auto& from = _from.GetVk();
    auto& commandBuffer = _commandBuffer.GetVk();
    auto& descriptorResource = m_DescriptorSet.GetVk();
    auto& pipeline = m_Pipeline.GetVk();
    auto& mesh=m_DeviceMesh.GetVk();
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

} // namespace Aether::UI