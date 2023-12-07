#pragma once
#include "Mesh.h"
#include <vector>
#include <filesystem>
#include "../Resource/ModelAsset.h"
namespace Aether
{
	class Model
	{
	public:
		Model(Ref<ModelAsset> modelAsset);
		~Model() {}
		Model(Model&&) = default;
		Model(const Model&) = delete;
		inline std::filesystem::path& GetPath() { return m_Path; }
		inline std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
		inline std::vector<Ref<Shader>>& GetShaders() { return m_Shaders; }
	private:
		std::filesystem::path m_Path;
		std::vector<Ref<Mesh>> m_Meshes;
		std::vector<Ref<Shader>> m_Shaders;
	};
}