#pragma once
#include "Aether/Render/Camera.h"
#include "Scene.h"
namespace Aether
{
    class SceneRenderer
    {
    public:
        SceneRenderer()=default;
        ~SceneRenderer()=default;
        static SceneRenderer& Get()
        {
            static SceneRenderer instance;
            return instance;
        }
        void Render(Scene& scene,PerspectiveCamera& camera);
        void Render(Scene& scene);
    private:
        void RenderSkybox(SkyboxComponent& sc, PerspectiveCamera& camera);
        void RenderObject(Scene& scene,PerspectiveCamera& camera,
            SkyboxComponent* sc);
    };
}