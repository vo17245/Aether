#include "Component.h"
#include "../Core/Config.h"
#include <filesystem>
Aether::MeshComponent::MeshComponent(Ref<ModelAsset>& modelAsset)
{
	for (auto& meshAsset : modelAsset->GetMeshes())
	{
		auto vb=VertexBuffer::Create(meshAsset.Vertices.data(), sizeof(Vertex) * meshAsset.Vertices.size());
		auto vbl = Vertex::CreateVertexBufferLayout();
		auto va = CreateRef<VertexArray>();
		va->SetData(*vb, vbl);
		std::filesystem::path resDir(Config::ResourcePath);
		std::filesystem::path filename = "Basic.shader";
		auto shader = Shader::CreateRefFromFile((resDir / filename).string().c_str());
	}
}
