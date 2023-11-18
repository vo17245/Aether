#pragma once
#include "Entity.h"
#include "Scene.h"
#include "Component.h"
#include <filesystem>
#include "../Render/Camera.h"
#include <optional>
namespace Aether
{
	
	struct DeserializationResult
	{
		DeserializationResult()=default;
		DeserializationResult(const DeserializationResult&) = default;
		DeserializationResult(DeserializationResult&&) = default;
		Ref<Scene> scene;
		Camera camera;
		std::string name;
	};
	class SceneSerializer
	{
	public:
		static bool WriteFile(Scene& scene, const std::string& name,
			const Camera& camera,const std::filesystem::path& path);
		static std::vector<std::byte> WriteMem(Scene& scene,const std::string& name,
			const Camera& camera);
		static std::optional<DeserializationResult> LoadFile(const std::filesystem::path& path);
		static std::optional<DeserializationResult> LoadMem(const std::vector<std::byte>& buf);
	};
}
