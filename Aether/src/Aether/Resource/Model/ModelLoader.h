#pragma once
#include "Aether/Core/Core.h"
#include "Aether/Resource/Model/Model.h"
#include "Model.h"
#include <optional>
#include <string>
#include <filesystem>
namespace Aether{
class ModelLoader
{
public:
    static std::optional<Ref<Model>> LoadFromFile(const std::filesystem::path& path);
};
}//namespace Aether