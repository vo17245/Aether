#pragma once

#include <memory>
#include "../Asset/MeshAsset.h"
#include "../Render/VertexArray.h"
#include "../Render/VertexBuffer.h"
#include "../Render/VertexBufferLayout.h"
#include "../Render/IndexBuffer.h"
#include "../Render/Texture2D.h"
#include "../Render/Shader.h"
#include "../Render/Renderer.h"
AETHER_NAMESPACE_BEGIN
class Mesh
{
public:
	Mesh(std::shared_ptr<MeshAsset> meshAsset);
	Mesh(const MeshAsset& meshAsset);
	~Mesh();
	void Draw(std::shared_ptr<Shader> shader)const;
	void Draw(std::shared_ptr<Shader> shader,Eigen::Matrix4f modelMatrix)const;
private:
	void CreateRendererObject(const MeshAsset& meshAsset);

private:
	std::shared_ptr<VertexArray> m_VA;
	std::shared_ptr<VertexBuffer> m_VB;
	std::shared_ptr<VertexBufferLayout> m_VBL;
	std::shared_ptr<IndexBuffer> m_IB;
	std::vector<std::shared_ptr<Texture2D>> m_Textures;
	GLDrawMode m_Mode;
public:
	std::shared_ptr<VertexArray> GetVertexArray() { return m_VA; }
	std::shared_ptr<VertexBuffer> GetVertexBuffer() { return m_VB; }
	std::shared_ptr<VertexBufferLayout> GetVertexBufferLayout() { return m_VBL; }
	std::shared_ptr<IndexBuffer> GetIndexBuffer() { return m_IB; }
private:
	//model matrix
	std::optional<Eigen::Vector3f> m_PositionMax;
	std::optional<Eigen::Vector3f> m_PositionMin;

};
AETHER_NAMESPACE_END

