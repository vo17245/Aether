#pragma once
#include "../Core/UUID.h"
#include "Eigen/Core"
#include <vector>
#include "../Render/Mesh.h"
#include "../Asset/ModelAsset.h"
#include "../Render/Light.h"
#include "../Render/Model.h"
namespace Aether
{
	struct IDComponent
	{
		UUID id;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& _id):id(_id){}
	};
	struct TagComponent
	{
		std::string tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& _tag)
			:tag(_tag) {}
	};
	struct TransformComponent
	{
		Eigen::Vector3f position;
		Eigen::Vector3f rotation;//旋转顺序为x y z
		Eigen::Vector3f scaling;
		Eigen::Matrix4f matrix;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent()
			:position(Eigen::Vector3f::Zero()),
			rotation(Eigen::Vector3f::Zero()),
			scaling(1,1,1) ,matrix(Eigen::Matrix4f::Identity()){}
		void CalculateMatrix();
	};
	struct VisualComponent
	{
		Ref<Model> model;
		VisualComponent(const VisualComponent&) = default;
		VisualComponent() = default;
		VisualComponent(Ref<Model> _model)
			:model(_model) {}
		
	};
	struct PointLightComponent
	{
		PointLight light;
		PointLightComponent(PointLightComponent&&)=default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent() = default;
		PointLightComponent(const Vec3& color, const Vec3& pos)
			:light(color,pos)
		{}
	};
}
