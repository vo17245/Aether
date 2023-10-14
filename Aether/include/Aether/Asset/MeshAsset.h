#pragma once

#include "Eigen/Core"
#include <vector>
#include "Vertex.h"
#include "Image.h"
#include <memory>
#include "TextureAsset.h"
#include "../Render/OpenGLApi.h"
AETHER_NAMESPACE_BEGIN
class MeshAsset
{
public:
	MeshAsset() 
		:m_Mode(GLDrawMode::TRIANGLES)
	{}
	MeshAsset(size_t vertexCount,size_t indexCount)
		:Vertices(vertexCount),Indices(indexCount),m_Mode(GLDrawMode::TRIANGLES)
	{}
	~MeshAsset(){}
	MeshAsset(MeshAsset&& mesh)noexcept
	{
		Vertices = std::move(mesh.Vertices);
		Indices = std::move(mesh.Indices);
		Textures = std::move(mesh.Textures);
		m_Mode = mesh.m_Mode;
	}
	MeshAsset(const MeshAsset& mesh)
	{
		Vertices = mesh.Vertices;
		Indices = mesh.Indices;
		Textures = mesh.Textures;
		m_Mode = mesh.m_Mode;
	}
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<std::shared_ptr<TextureAsset>> Textures;
	GLDrawMode m_Mode;
};
AETHER_NAMESPACE_END