#pragma once
#include "../Core/UUID.h"
#include "Eigen/Core"
#include <vector>
#include "../Render/Mesh.h"
#include "../Asset/ModelAsset.h"
#include "../Render/Light.h"
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
		TagComponent(const std::string& tag)
			:Tag(tag) {}
	};
	struct TransformComponent
	{
		Eigen::Vector3f Position;
		Eigen::Vector3f Rotation;//旋转顺序为x y z
		Eigen::Vector3f Scaling;
		Eigen::Matrix4f Matrix;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent()
			:Position(Eigen::Vector3f::Zero()),
			Rotation(Eigen::Vector3f::Zero()),
			Scaling(1,1,1) ,Matrix(Eigen::Matrix4f::Identity()){}
		void CalculateMatrix();
	};
	struct VisualComponent
	{
		std::vector<Ref<Mesh>> Meshes;
		std::vector<Ref<Shader>> Shaders;
		VisualComponent(const VisualComponent&) = default;
		VisualComponent() = default;
		VisualComponent(Ref<ModelAsset>& modelAsset);
		
	};
	struct PointLightComponent
	{
		PointLight Light;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent() = default;
		PointLightComponent(const Vec3& color, const Vec3& pos)
			:Light(color,pos)
		{}
	};
}
