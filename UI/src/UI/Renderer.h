#pragma once
#include "Render/RenderApi.h"
#include "Quad.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"       
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "QuadArrayMesh.h"
namespace Aether::UI
{
    class Renderer
    {
    public:

        void Begin(DeviceRenderPass& renderPass,DeviceFrameBuffer& frameBuffer);
        void DrawQuad(const Quad& quad);
        void End(DeviceCommandBuffer& _commandBuffer);
        /**
        * @brief use renderPass to init pipeline in vulkan backend
        *       in vulkan The pipeline must only be used with a render pass instance compatible with the one provided. 
        *       See Render Pass Compatibility for more information.
        */
        static std::expected<Renderer,std::string> Create(DeviceRenderPass& renderPass);
    private:
        bool CreateDescriptorPool();
        // ====== basic
        DevicePipeline m_BasicPipeline;// only draw quad with color
        DeviceDescriptorSet m_BasicDescriptorSet;
        DeviceShader m_BasicShader;
        bool CreateBasicPipeline(DeviceRenderPass& _renderPass);
        bool CreateBasicDescriptorSet();
        bool CreateBasicShader();
        //============
        DevicePipeline m_TexturePipeline;// draw quad with texture
        DeviceDescriptorPool m_DescriptorPool;
    private:
        QuadArrayMesh m_BasicQuadArray;// quad with color
    private:
        DeviceFrameBuffer* m_FrameBuffer=nullptr;
        DeviceRenderPass* m_RenderPass=nullptr;
    private:
        DeviceCommandBuffer m_TransformCommandBuffer;// for vertex data transform
    }; 
}