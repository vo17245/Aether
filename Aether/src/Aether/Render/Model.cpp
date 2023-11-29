#include "Model.h"

Aether::Model::Model(Ref<ModelAsset> modelAsset)
	:m_Path(modelAsset->GetPath())
{
	for (auto& meshAsset : modelAsset->GetMeshes())
	{
		auto vb = VertexBuffer::Create(meshAsset.Vertices.data(), sizeof(Vertex) * meshAsset.Vertices.size());
		auto vbl = Vertex::CreateVertexBufferLayout();
		auto va = CreateRef<VertexArray>();
		auto ib = CreateRef<IndexBuffer>(meshAsset.Indices.data(), meshAsset.Indices.size());
		va->SetData(*vb, vbl);
		m_Meshes.emplace_back(CreateRef<Mesh>(va, vb, ib));
		m_Shaders.push_back(Shader::Premake::GetBasic());
	}
}
