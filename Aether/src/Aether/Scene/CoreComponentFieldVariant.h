#pragma once
#include <variant>

#include "../Core/Config.h"
#include "../Core/UUID.h"
#include "../Render/Light.h"
#include "../Render/Transform.h"
#include "Aether/Core/Math.h"
#include "Aether/Render/Camera.h"
#include "Aether/Render/CubeMap.h"
#include "Aether/Render/Texture2D.h"
#include "Aether/Resource/Model/Model.h"
#include "Eigen/Core"
#include <filesystem>
#include <vector>
namespace Aether {
namespace Reflection {
using CoreComponentFieldVariant =
    std::variant<std::string, Ref<Texture2D>, UUID, Ref<CubeMap>,
                 std::optional<std::string>, bool, PerspectiveCamera,
                 Ref<Model>, ::Eigen::Vector3f, Eigen::Matrix4f, float,
                 PointLight>;

}
} // namespace Aether
