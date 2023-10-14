#pragma once

#include "Eigen/Core"
#include "../Render/VertexBufferLayout.h"
AETHER_NAMESPACE_BEGIN
class Vertex
{
public:
	Vertex(const Eigen::Vector3f& position, const Eigen::Vector3f& normal, const Eigen::Vector2f& texCoords)
		:Position(position),Normal(normal),TexCoords(texCoords){}
	~Vertex(){}
	Vertex(){}
	Vertex(const Vertex& vertex)
		:Position(vertex.Position),Normal(vertex.Normal),TexCoords(vertex.TexCoords){}
public:
	Eigen::Vector3f Position;
	Eigen::Vector3f Normal;
	Eigen::Vector2f TexCoords;
public:
	static VertexBufferLayout& GetVertexBufferLayout();
	
};
AETHER_NAMESPACE_END