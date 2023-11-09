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
		Eigen::Vector3d Translation;
		Eigen::Vector3d Rotation;//旋转顺序为x y z
		Eigen::Vector3d Scaling;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent()
			:Translation(Eigen::Vector3d::Zero()),
			Rotation(Eigen::Vector3d::Zero()),
			Scaling(1,1,1) {}
	};
	struct MeshComponent
	{
		std::vector<Ref<Mesh>> Meshes;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent() = default;
		MeshComponent(Ref<ModelAsset>& modelAsset);
	};
}
