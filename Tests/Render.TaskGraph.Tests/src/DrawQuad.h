#pragma once
#include <Core/Core.h>
#include <Render/RenderApi.h>
using namespace Aether;
class DrawQuad
{
public:
    /**
     * @param affine  use in screen space
    */
    static std::expected<DrawQuad, std::string> Create(const std::string& vs,const std::string& fs,const Mat2x3f& affine);
    /**
     * @note caller need to begin render pass before calling this function
    */
    void Render(DeviceCommandBuffer& commandBuffer);
private:
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceMesh m_Mesh;
};