#pragma once
#include "Render/RenderApi.h"
#include "Quad.h"
#include "Render/RenderApi/DeviceCommandBuffer.h"       
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "QuadArrayMesh.h"
#include "Render/Temporary.h"
namespace Aether::UI
{
    class Renderer
    {
    public:
        Renderer(const Renderer&)=delete;
        Renderer(Renderer&&)=default;
        void Begin(DeviceRenderPassView renderPass,DeviceFrameBufferView frameBuffer,Vec2f screenSize);
        void DrawQuad(const Quad& quad);
        void End(DeviceCommandBufferView _commandBuffer);
        /**
        * @brief use renderPass to init pipeline in vulkan backend
        *       in vulkan The pipeline must only be used with a render pass instance compatible with the one provided. 
        *       See Render Pass Compatibility for more information.
        */
        static std::expected<Renderer,std::string> Create(DeviceRenderPassView renderPass);
        // for unit test
        static Renderer CreateEmpty();
        inline void SetScreenSize(Vec2f screenSize)
        {
            m_ScreenSize=screenSize;
        }
        Mat4 CalculateModelMatrix();

    private:
        Renderer()=default;
        bool CreateDescriptorPool();
        // ====== basic
        struct BasicUniformBlock
        {
            Mat4 model=Mat4::Identity();
            Mat4 view=Mat4::Identity();
        };
        static_assert(sizeof(BasicUniformBlock)==4*16*2);
        DeviceBuffer m_BasicUboStaging;
        DeviceBuffer m_BasicUboBuffer;
        DevicePipeline m_BasicPipeline;// only draw quad with color
        DevicePipelineLayout m_BasicPipelineLayout;
        DeviceDescriptorSet m_BasicDescriptorSet;
        DeviceShader m_BasicShader;
        BasicUniformBlock m_BasicUboLocalBuffer;
        bool CreateBasicPipeline(DeviceRenderPassView _renderPass);
        bool CreateBasicDescriptorSet();
        bool CreateBasicShader();
        bool CreateBasicBuffer();
        bool UploadBasicBuffer();

        //============
        DevicePipeline m_TexturePipeline;// draw quad with texture
        DeviceDescriptorPool m_DescriptorPool;
        /**
        * @brief create model matrix for quad, translate from screen space to NDC space
        */
    private:
        QuadArrayMesh m_BasicQuadArray;// quad with color
    private:
        DeviceFrameBufferView m_FrameBuffer;
        DeviceRenderPassView m_RenderPass;
        Vec2f m_ScreenSize=Vec2f(0,0);
    private:
        DeviceCommandBuffer m_TransformCommandBuffer;// for vertex data transform
    private:
        Temporary<DeviceMesh> m_Temporary;
    }; 
}