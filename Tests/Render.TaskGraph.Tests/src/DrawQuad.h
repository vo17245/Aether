#pragma once
#include <Core/Core.h>
#include <Render/RenderApi.h>
using namespace Aether;
class DrawQuad// just for test
{
public:
    /**
     * @param affine  use in screen space
    */
    static std::expected<DrawQuad, std::string> Create(const std::string& vs,const std::string& fs,const Mat2x3f& affine,
    PixelFormat colorAttachPixelFormat,bool clear);
    /**
     * @note caller need to begin render pass before calling this function
    */
    void Render(DeviceCommandBufferView& commandBuffer);
    void SetScreenSize(const Vec2u& size)
    {
        m_ScreenSize=size;
    }
private:
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    DeviceMesh m_Mesh;
    Vec2u m_ScreenSize;
};