#include "Aether/Scene/SceneRenderer.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/ShaderSource.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Resource/ModelCache.h"
#include "Aether/Scene/Component.h"
#include "Aether/Render/ShaderCache.h"
namespace Aether
{
    void SceneRenderer::Render(Scene& scene,PerspectiveCamera& camera)
    {
        // camera
        camera.CalculateProjection();
        camera.CalculateView();
        // skybox ?
        SkyboxComponent* skybox=nullptr;
        auto view = scene.GetAllEntitiesWith<SkyboxComponent>();
        for (const auto& [entity, sc] : view.each())
        {
            skybox = &sc;
            break;
        }
        if (skybox)
        {
            RenderSkybox(*skybox,camera);
        }
        // obj pass
        RenderObject(scene,camera,skybox);

    }
    void SceneRenderer::RenderSkybox(SkyboxComponent& sc,PerspectiveCamera& camera)
    {
        GLCall(glDepthFunc(GL_LEQUAL));
        BuiltinShaderSignature signature(BuiltinShaderType::SKYBOX,0);
        auto shader=ShaderCache::GetInstance().GetShader(signature);
        AETHER_ASSERT(shader&&"failed to get skybox shader");
        shader->Bind();
        sc.envMap->Bind(0);
        Mat4 view = camera.GetView();
        Mat4 proj = camera.GetProjection();
        shader->SetMat4f("u_View", view);
        shader->SetMat4f("u_Projection", proj);
        shader->SetInt("u_EnvMap", 0);
        sc.model->Render();
    }
    void SceneRenderer::RenderObject(Scene& scene,PerspectiveCamera& camera,
        SkyboxComponent* sc)
    {

        auto view = scene.GetAllEntitiesWith<MeshComponent, PbrMeterialComponent>();
        for (const auto& [entity, mc, pmc] : view.each())
        {
            //no model
            if(!mc.model && !mc.filePath)
                continue;
            //model not loaded,try to load model
            mc.model=ModelCache::LoadFromFile(mc.filePath.value());
            if(!mc.model)//failed to load model
                continue;
            // shader macro
            uint32_t macro=0;
            if (sc)
            {
                macro |= (uint32_t)BuiltinShaderPbrMacro::IBL;
            }
            if (pmc.albedoMap)
            {
                macro |= (uint32_t)BuiltinShaderPbrMacro::USE_ALBEDO_TEX;
            }
            if (pmc.metallicMap)
            {
                macro |= (uint32_t)BuiltinShaderPbrMacro::USE_METALLIC_TEX;
            }
            if (pmc.roughnessMap)
            {
                macro |= (uint32_t)BuiltinShaderPbrMacro::USE_ROUGHNESS_TEX;
            }
            // get shader
            BuiltinShaderSignature signature(BuiltinShaderType::PBR, macro);
            auto shader = ShaderCache::GetInstance().GetShader(signature);
            AETHER_ASSERT(shader && "failed to get shader");
            shader->Bind();
            //set camera uniform
            Mat4 modelMatrix = Mat4::Identity();
            shader->SetMat4f("u_Model", modelMatrix);
            Mat4 normalMatrix = modelMatrix.inverse();//rendering in world space
            normalMatrix.transposeInPlace();
            shader->SetMat4f("u_NormalMatrix", normalMatrix);
            Mat4 modelView = camera.GetView() * modelMatrix;
            Mat4 mvp = camera.GetProjection() * modelView;
            shader->SetMat4f("u_ModelViewProjection", mvp);
            shader->SetVec3f("u_CamPos", camera.GetPosition());
            //set light
            shader->SetInt("u_LightCnt", 0);
            // bind tex and set material uniform
            int tex_index = 0;
            if (macro & (uint32_t)BuiltinShaderPbrMacro::IBL)
            {
                //prefilter map
                sc->prefilterMap->Bind(tex_index);
                shader->SetInt("u_PrefilterMap",tex_index);
                ++tex_index;
                //irradiance map
                sc->irradianceMap->Bind(tex_index);
                shader->SetInt("u_IBL_DiffuseMap", tex_index);
                ++tex_index;
                //brdf LUT
                sc->brdfLUT->Bind(tex_index);
                shader->SetInt("u_BrdfLUT", tex_index);
                ++tex_index;
            }
            if (macro & (uint32_t)BuiltinShaderPbrMacro::USE_ALBEDO_TEX)
            {
                pmc.albedoMap->Bind(tex_index);
                shader->SetInt("u_AlbedoTex", tex_index);
                ++tex_index;
            }
            else
            {
                shader->SetVec3f("u_Albedo", pmc.albedo);
            }
            if (macro & (uint32_t)BuiltinShaderPbrMacro::USE_AO_TEX)
            {
                pmc.aoMap->Bind(tex_index);
                shader->SetInt("u_AoTex", tex_index);
                ++tex_index;
            }
            else
            {
                shader->SetFloat("u_Ao", pmc.ao);
            }

            if (macro & (uint32_t)BuiltinShaderPbrMacro::USE_METALLIC_TEX)
            {
                pmc.metallicMap->Bind(tex_index);
                shader->SetInt("u_MetallicTex", tex_index);
                ++tex_index;
            }
            else
            {
                shader->SetFloat("u_Metallic", pmc.metallic);
            }
            if (macro & (uint32_t)BuiltinShaderPbrMacro::USE_ROUGHNESS_TEX)
            {
                pmc.roughnessMap->Bind(tex_index);
                shader->SetInt("u_RoughnessTex", tex_index);
                ++tex_index;
            }
            else
            {
                shader->SetFloat("u_Roughness", pmc.roughness);
            }
            //draw
            mc.model->Render();
        }

    }
    void SceneRenderer::Render(Scene& scene)
    {
        PerspectiveCameraComponent* primaryCamera=nullptr;
        auto view = scene.GetAllEntitiesWith<PerspectiveCameraComponent>();
        for (const auto& [entity, pc] : view.each())
        {
            if (pc.isPrimary)
            {
                primaryCamera = &pc;
                break;
            }
        }
        AETHER_ASSERT(primaryCamera&&"no primary camera");
        Render(scene,primaryCamera->camera);
    }
}//namespace Aether