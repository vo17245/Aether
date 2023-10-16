#include "Model.h"
AETHER_NAMESPACE_BEGIN
Model::Model(std::shared_ptr<ModelAsset> modelAsset)
{
	for (size_t i = 0; i < modelAsset->GetMeshes().size(); i++)
	{
		auto mesh = std::make_shared<Mesh>(modelAsset->GetMeshes()[i]);
		m_Meshes.push_back(mesh);
	}

}

Model::~Model()
{
}

void Model::Draw(std::shared_ptr<Shader> shader)const
{
	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		m_Meshes[i]->Draw(shader);
	}
}
void Model::Draw(std::shared_ptr<Shader> shader, Eigen::Matrix4f modelMatrix) const
{
	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		m_Meshes[i]->Draw(shader,modelMatrix);
	}
}
AETHER_NAMESPACE_END