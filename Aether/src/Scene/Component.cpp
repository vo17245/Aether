#include "Component.h"
#include "../Core/Config.h"
#include <filesystem>
#include "../Render/Transform.h"
Aether::VisualComponent::VisualComponent(Ref<ModelAsset>& modelAsset)
{
	
	for (auto& meshAsset : modelAsset->GetMeshes())
	{
		auto vb=VertexBuffer::Create(meshAsset.Vertices.data(), sizeof(Vertex) * meshAsset.Vertices.size());
		auto vbl = Vertex::CreateVertexBufferLayout();
		auto va = CreateRef<VertexArray>();
		auto ib = CreateRef<IndexBuffer>(meshAsset.Indices.data(), meshAsset.Indices.size());
		va->SetData(*vb, vbl);
		Meshes.emplace_back(CreateRef<Mesh>(va, vb, ib));
		Shaders.push_back(Shader::Premake::GetBasic());
	}
}

void Aether::TransformComponent::CalculateMatrix()
{
	Matrix = Transform::Translation(Translation)*Transform::Rotation(Rotation)*Transform::Scale(Scaling);
}
