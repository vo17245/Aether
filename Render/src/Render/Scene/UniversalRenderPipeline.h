#pragma once
#include <Render/RenderApi.h>
#include <Core/Borrow.h>
namespace Aether::Render::Scene
{
    struct ShaderLocation
    {
        uint32_t set;
        uint32_t binding;
    };

    struct Material
    {

        std::vector<Borrow<DeviceTexture>> textures;
        std::vector<ShaderLocation> textureLocations;
        std::vector<Borrow<DeviceBuffer>> buffers;
        std::vector<ShaderLocation> bufferLocations;
        Borrow<DevicePipeline> shader;
    };
    struct Mesh
    {
        Borrow<DeviceMesh> deviceMesh;
        Borrow<Material> material;
    };
    class UniversalRenderPipeline
    {
    public:
        // order: skybox -> main pass -> post effect
        // just record command , not submit in this class
        void BeginSkyboxPass(DeviceRenderPass& renderPass,DeviceFrameBuffer& frameBuffer,DeviceCommandBuffer& commandBuffer);
        void SkyboxPass(Mesh& mesh);
        void EndSkyboxPass(DeviceCommandBuffer& commandBuffer);
        // render scene objects
        // call for each object in the scene
        void BeginMainPass(DeviceRenderPass& renderPass,DeviceFrameBuffer& frameBuffer,DeviceCommandBuffer& commandBuffer);
        void MainPass(Mesh& mesh);
        void EndMainPass(DeviceCommandBuffer& commandBuffer);
        // post effect pass
        void BeginPostEffect(DeviceRenderPass& renderPass,DeviceFrameBuffer& frameBuffer,DeviceCommandBuffer& commandBuffer);
        void PostEffectPass(Mesh& mesh);
        void EndPostEffectPass(DeviceCommandBuffer& commandBuffer);
    private:
    };
}