#pragma once
#include "Backend/Vulkan/Pipeline.h"
#include "Backend/Vulkan/Pipeline.h"
#include <variant>
#include <Render/Mesh/VertexLayout.h>
#include "Shader.h"
namespace Aether::rhi
{
struct PipelineDesc
{
    VertexLayout vertexLayout;
    std::vector<PixelFormat> colorAttachmentFormats;
    std::optional<PixelFormat> depthAttachmentFormat;
    std::optional<PixelFormat> stencilAttachmentFormat;
    VertexShader* vertexShader = nullptr;
    PixelShader* pixelShader = nullptr;
    bool enableBlend = false;
    bool enableDepthTest = false;
};
class Pipeline
{
public:
    Pipeline() = default;
    Pipeline(Pipeline&& other) noexcept = default;
    Pipeline& operator=(Pipeline&& other) noexcept = default;
    Pipeline(vk::GraphicsPipeline&& pipeline) : m_Pipeline(std::move(pipeline))
    {
    }
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

public:
    bool Empty() const
    {
        return m_Pipeline.index() == 0;
    }
    vk::GraphicsPipeline& GetVk()
    {
        return std::get<vk::GraphicsPipeline>(m_Pipeline);
    }
    const vk::GraphicsPipeline& GetVk() const
    {
        return std::get<vk::GraphicsPipeline>(m_Pipeline);
    }
    operator bool() const
    {
        return !Empty();
    }
    static Pipeline Create(const PipelineDesc& desc);

private:
    std::variant<std::monostate, vk::GraphicsPipeline> m_Pipeline;
};
} // namespace Aether::rhi