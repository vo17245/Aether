#pragma once
#include "../Core/UUID.h"
#include "Eigen/Core"
#include <vector>
#include "../Render/Mesh.h"
#include "../Asset/ModelAsset.h"
namespace Aether
{
	struct IDComponent
	{
		UUID ID;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};
	struct TagComponent
	{
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
	};
	struct TransformComponent
	{
		Eigen::Vector3f Translation;
		Eigen::Vector3f Rotation;//旋转顺序为x y z
		Eigen::Vector3f Scaling;
		Eigen::Matrix4f Matrix;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent()
			:Translation(Eigen::Vector3f::Zero()),
			Rotation(Eigen::Vector3f::Zero()),
			Scaling(1,1,1) ,Matrix(Eigen::Matrix4f::Identity()){}
		void CalculateMatrix();
	};
	struct MeshComponent
	{
		std::vector<Ref<Mesh>> Meshes;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent() = default;
		MeshComponent(Ref<ModelAsset>& modelAsset);
		
	};
}
