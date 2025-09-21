#pragma once
#include "PipelineLayout.h"
#include <memory>
#include <span>
#include <string_view>
#include "Traits.h"
#include "Render/Mesh/VertexBufferLayout.h"
#include "vulkan/vulkan_core.h"
#include "PipelineLayout.h"
#include "ShaderModule.h"
#include "RenderPass.h"
namespace Aether {
namespace vk {

class GraphicsPipeline
{
public:
    class Builder
    {
    public:
        Builder(
            const RenderPass& renderPass,
            const PipelineLayout& layout) :

            renderPass(renderPass),
            layout(layout)
        {
        }
        template <AreAllVertexBufferLayout... VertexBufferLayouts>
        Builder& PushVertexBufferLayouts(const VertexBufferLayouts&... vertexBufferLayout)
        {
            // vertexBindingDescription
            auto vertexBindingDescriptionArray = make_vertex_input_binding_description_array(vertexBufferLayout...);
            m_VertexBindingDescriptions.insert(m_VertexBindingDescriptions.end(),
                                               vertexBindingDescriptionArray.begin(),
                                               vertexBindingDescriptionArray.end());
            // vertexAttributeDescription
            auto vertexAttributeDescriptionArray = merge_vertex_attribute_description(vertexBufferLayout...);
            m_AttributeDescriptions.insert(m_AttributeDescriptions.end(),
                                           vertexAttributeDescriptionArray.begin(),
                                           vertexAttributeDescriptionArray.end());
            return *this;
        }
        Builder& PushVertexBufferLayouts(const VertexBufferLayout* layouts, size_t size)
        {
            for (size_t i = 0; i < size; i++)
            {
                auto vertexBindingDescriotion = layouts[i].CreateVulkanBindingDescription();
                m_VertexBindingDescriptions.emplace_back(vertexBindingDescriotion);
                auto vertexAttributeDescriptions = layouts[i].CreateVulkanAttributeDescriptions();
                m_AttributeDescriptions.insert(m_AttributeDescriptions.end(),
                                               vertexAttributeDescriptions.begin(),
                                               vertexAttributeDescriptions.end());
            }
            return *this;
        }
        Builder& PushVertexBufferLayouts(const std::vector<VertexBufferLayout>& layouts)
        {
            return PushVertexBufferLayouts(layouts.data(), layouts.size());
        }
        Builder& PushVertexBufferLayouts(const std::span<VertexBufferLayout> layouts)
        {
            return PushVertexBufferLayouts(layouts.data(), layouts.size());
        }
        std::optional<GraphicsPipeline> Build();
        Scope<GraphicsPipeline> BuildScope()
        {
            auto opt = Build();
            if (!opt)
            {
                return nullptr;
            }
            return CreateScope<GraphicsPipeline>(std::move(opt.value()));
        }
        Ref<GraphicsPipeline> BuildRef()
        {
            auto opt = Build();
            if (!opt)
            {
                return nullptr;
            }
            return ::Aether::CreateRef<GraphicsPipeline>(std::move(opt.value()));
        }

        Builder& EnableDepthTest()
        {
            m_DepthTestEnable = true;
            return *this;
        }

        Builder& SetDepthTestCompareFunc(VkCompareOp compareOp)
        {
            m_DepthTestCompareOp = compareOp;
            return *this;
        }
        Builder& EnableBlend()
        {
            assert(m_InColorAttachment && "EnableBlend must be called between BeginColorAttachment and EndColorAttachment");
            m_ColorAttachmentConfigs.back().enableBlend = true;
            return *this;
        }
        Builder& AddVertexStage(ShaderModule& shaderModule, const char* entryPoint)
        {
            VkPipelineShaderStageCreateInfo stage{};
            stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
            stage.module = shaderModule.GetHandle();
            stage.pName = entryPoint;
            m_Stages.push_back(stage);
            return *this;
        }
        Builder& AddFragmentStage(ShaderModule& shaderModule, const char* entryPoint)
        {
            VkPipelineShaderStageCreateInfo stage{};
            stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            stage.module = shaderModule.GetHandle();
            stage.pName = entryPoint;
            m_Stages.push_back(stage);
            return *this;
        }
        Builder& BeginColorAttachment()
        {
            assert(!m_InColorAttachment && "Already in color attachment");
            m_InColorAttachment = true;
            m_ColorAttachmentConfigs.emplace_back();
            return *this;
        }
        Builder& EndColorAttachment()
        {
            assert(m_InColorAttachment && "Not in color attachment");
            m_InColorAttachment = false;
            return *this;
        }

    private:
        const RenderPass& renderPass;
        const PipelineLayout& layout;
        std::vector<VkVertexInputBindingDescription> m_VertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
        bool m_DepthTestEnable = false;
        VkCompareOp m_DepthTestCompareOp = VK_COMPARE_OP_LESS;
        std::vector<VkPipelineShaderStageCreateInfo> m_Stages;
    private:
        struct ColorAttachmentConfig
        {
            bool enableBlend=false;
        };
        std::vector<ColorAttachmentConfig> m_ColorAttachmentConfigs;
        bool m_InColorAttachment=false;
    };

public:
    ~GraphicsPipeline();
    VkPipeline GetHandle() const
    {
        return m_Pipeline;
    }
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    GraphicsPipeline(GraphicsPipeline&& other) noexcept
    {
        m_Pipeline = other.m_Pipeline;
        other.m_Pipeline = VK_NULL_HANDLE;
    }
    GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

private:
    VkPipeline m_Pipeline;
    GraphicsPipeline(VkPipeline pipeline) :
        m_Pipeline(pipeline)
    {
    }
};
}
} // namespace Aether::vk