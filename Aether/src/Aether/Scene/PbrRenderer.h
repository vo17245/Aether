#pragma once
#include "Aether/Render/Camera.h"
#include "Scene.h"
namespace Aether {
class PbrRenderer
{
public:
    PbrRenderer() = default;
    ~PbrRenderer() = default;

    void Render(Scene& scene, PerspectiveCamera& camera);
    void Render(Scene& scene);

private:
    void RenderSkybox(SkyboxComponent& sc, PerspectiveCamera& camera);
    void RenderObject(Scene& scene, PerspectiveCamera& camera,
                      SkyboxComponent* sc);
};
} // namespace Aether