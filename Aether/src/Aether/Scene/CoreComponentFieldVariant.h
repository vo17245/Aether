#pragma once
#include <variant>

#include "Aether/Render/Texture2D.h"
#include <filesystem>
#include "Aether/Core/Math.h"
#include "../Render/Light.h"
#include "../Core/Config.h"
#include "Aether/Render/CubeMap.h"
#include "../Render/Transform.h"
#include "Aether/Render/Camera.h"
#include <vector>
#include "Eigen/Core"
#include "../Core/UUID.h"
#include "Aether/Resource/Model/Model.h"
namespace Aether { namespace Reflection {
using CoreComponentFieldVariant = std::variant<PointLight, Ref<Model>, std::optional<std::string>, UUID, ::Eigen::Vector3f, PerspectiveCamera, Ref<Texture2D>, std::string, float, Eigen::Matrix4f, Ref<CubeMap>, bool>;

}} // namespace Aether::Reflection
