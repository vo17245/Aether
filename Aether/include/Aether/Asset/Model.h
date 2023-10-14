#pragma once

#include "ModelAsset.h"
#include <memory>
#include "../Render/VertexArray.h"
#include "../Render/IndexBuffer.h"
#include "Mesh.h"
#include <vector>
AETHER_NAMESPACE_BEGIN
class Model
{
public: 
	Model(std::shared_ptr<ModelAsset> modelAsset);
	~Model();
	void Draw(std::shared_ptr<Shader> shader)const;
private:
	std::vector<std::shared_ptr<Mesh>> m_Meshes;
};
AETHER_NAMESPACE_END