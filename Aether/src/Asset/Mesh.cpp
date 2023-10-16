#include "Mesh.h"
#include <unordered_map>
#include "../Render/OpenGLApi.h"
#include "../Render/Math.h"
AETHER_NAMESPACE_BEGIN
Mesh::Mesh(std::shared_ptr<MeshAsset> meshAsset)
	:m_Mode(GLDrawMode::TRIANGLES)
{

	CreateRendererObject(*meshAsset);
}

Mesh::Mesh(const MeshAsset& meshAsset)
	:m_Mode(GLDrawMode::TRIANGLES)
{
	
	CreateRendererObject(meshAsset);
}

Mesh::~Mesh()
{
}

void Mesh::Draw(std::shared_ptr<Shader> shader)const
{
	m_VA->Bind();
	m_VB->Bind();
	m_IB->Bind();
	shader->Bind();
	std::unordered_map<std::string, size_t> cnt;
	for (size_t i = 0; i < m_Textures.size(); i++)
	{
		m_Textures[i]->Bind();
		if (cnt.find(m_Textures[i]->GetTypeName()) == cnt.end())
		{
			cnt[m_Textures[i]->GetTypeName()] = 0;
		}
		shader->SetInt(m_Textures[i]->GetTypeName() + "_" + std::to_string(cnt[m_Textures[i]->GetTypeName()]), m_Textures[i]->GetSlot());
	}
	OpenGLApi::DrawElements(*m_VA, *m_IB);
	
}

void Mesh::Draw(std::shared_ptr<Shader> shader, Eigen::Matrix4f modelMatrix) const
{
	// move position to origin 
	if (m_PositionMax && m_PositionMin)
	{
		modelMatrix = modelMatrix * Math::Translation(
			-(m_PositionMin.value().x() +m_PositionMax.value().x())/2,
			-(m_PositionMin.value().y() + m_PositionMax.value().y()) / 2,
			-(m_PositionMin.value().z() + m_PositionMax.value().z()) / 2
		);
	}
	shader->Bind();
	std::unordered_map<std::string, size_t> cnt;
	for (size_t i = 0; i < m_Textures.size(); i++)
	{
		m_Textures[i]->Bind();
		if (cnt.find(m_Textures[i]->GetTypeName()) == cnt.end())
		{
			cnt[m_Textures[i]->GetTypeName()] = 0;
		}
		shader->SetInt(m_Textures[i]->GetTypeName() + "_" + std::to_string(cnt[m_Textures[i]->GetTypeName()]), m_Textures[i]->GetSlot());
	}
	Renderer::Submit(*m_VA, *m_IB, *shader, modelMatrix);
}

void Mesh::CreateRendererObject(const MeshAsset& meshAsset)
{
	m_PositionMax = meshAsset.PositionMax;
	m_PositionMin = meshAsset.PositionMin;
	m_VA = std::make_shared<VertexArray>();
	m_VB = std::make_shared<VertexBuffer>(meshAsset.Vertices.data(), meshAsset.Vertices.size() * sizeof(Vertex));
	m_VBL = std::make_shared<VertexBufferLayout>(Vertex::CreateVertexBufferLayout());
	m_VA->SetData(*m_VB, *m_VBL);
	m_IB = std::make_shared<IndexBuffer>(meshAsset.Indices.data(), meshAsset.Indices.size());
	for (size_t i = 0; i < meshAsset.Textures.size(); i++)
	{
		auto texture = std::make_shared<Texture2D>(meshAsset.Textures[i]->GetImage(), static_cast<uint32_t>(i));
		texture->SetTypeName(meshAsset.Textures[i]->GetTypeName());
		m_Textures.push_back(texture);
	}
	m_Mode = meshAsset.m_Mode;
}
AETHER_NAMESPACE_END