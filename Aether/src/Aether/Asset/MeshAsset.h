#pragma once

#include "Eigen/Core"
#include <vector>
#include "Vertex.h"
#include "Image.h"
#include <memory>
#include "TextureAsset.h"
#include "../Render/OpenGLApi.h"
#include <optional>
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
		PositionMax = std::move(mesh.PositionMax);
		PositionMin = std::move(mesh.PositionMin);
	}
	MeshAsset(const MeshAsset& mesh)
	{
		Vertices = mesh.Vertices;
		Indices = mesh.Indices;
		Textures = mesh.Textures;
		m_Mode = mesh.m_Mode;
		PositionMax = mesh.PositionMax;
		PositionMin = mesh.PositionMin;
	}
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<std::shared_ptr<TextureAsset>> Textures;
	std::optional<Eigen::Vector3f> PositionMax;
	std::optional<Eigen::Vector3f> PositionMin;
	GLDrawMode m_Mode;
	void CalculateBoundingBox();
};
AETHER_NAMESPACE_END