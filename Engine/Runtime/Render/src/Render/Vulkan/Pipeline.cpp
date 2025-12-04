#include "Pipeline.h"
#include "vulkan/vulkan_core.h"
#include "GlobalRenderContext.h"
#include "GlobalPipelineCache.h"
namespace Aether {
namespace vk {

std::optional<GraphicsPipeline> GraphicsPipeline::Builder::Build()
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // vertexBindingDescription
    vertexInputInfo.vertexBindingDescriptionCount = m_VertexBindingDescriptions.size();
    // vertexAttributeDescription
    vertexInputInfo.vertexAttributeDescriptionCount = m_AttributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = m_VertexBindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
    for(auto& config:m_ColorAttachmentConfigs)
    {   
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if (config.enableBlend)
        {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;           // 源颜色因子
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // 目标颜色因子
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                            // 颜色混合操作
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;                 // 源Alpha因子
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;                // 目标Alpha因子
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                            // Alpha混合操作
        }
        else
        {
            colorBlendAttachment.blendEnable = VK_FALSE;
        }
        colorBlendAttachments.push_back(colorBlendAttachment);
    }
    

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    // enable depth test
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    if (m_DepthTestEnable)
    {
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = m_DepthTestCompareOp;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
    }

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    if (m_EnableDynamicDepthCompareOp)
    {
        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT);
    }
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = m_Stages.size();
    pipelineInfo.pStages = m_Stages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout.GetHandle();
    if(renderPass)
    {
        pipelineInfo.renderPass = renderPass->GetHandle();
    }
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(GRC::GetDevice(), GlobalPipelineCache::Get(), 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return GraphicsPipeline(graphicsPipeline);
}
GraphicsPipeline::~GraphicsPipeline()
{
    if (m_Pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(GRC::GetDevice(), m_Pipeline, nullptr);
}
GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept
{
    if (this != &other)
    {
        if (m_Pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(GRC::GetDevice(), m_Pipeline, nullptr);
        m_Pipeline = other.m_Pipeline;
        other.m_Pipeline = VK_NULL_HANDLE;
    }
    return *this;
}
}
} // namespace Aether::vk