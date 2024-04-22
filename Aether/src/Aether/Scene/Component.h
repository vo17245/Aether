#pragma once
#include "../Core/UUID.h"
#include "Aether/Core/Math.h"
#include "Aether/Render/Texture2D.h"
#include "Eigen/Core"
#include <vector>
#include "../Render/Light.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Render/CubeMap.h"
#include "Aether/Render/Camera.h"

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
	
	struct PointLightComponent
	{
		PointLight light;
		PointLightComponent(PointLightComponent&&)=default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent() = default;
		PointLightComponent(const Vec3& color, const Vec3& pos)
			:light(color,pos)
		{}
		PointLightComponent& operator=(const PointLightComponent& plc)
		{
			light=plc.light;
			return *this;
		}
	};
	struct MeshComponent
	{
		Ref<Model> model;
		std::optional<std::string> filePath;
	};
	struct SkyboxComponent
	{
		Ref<Model> model;
		Ref<CubeMap> envMap;
		Ref<CubeMap> irradianceMap;
		Ref<CubeMap> prefilterMap;
		Ref<Texture2D> brdfLUT;
		SkyboxComponent(const Ref<Model>& _model,
				const Ref<CubeMap>& _envMap,
				const Ref<CubeMap>& _irradianceMap,
				const Ref<CubeMap>& _prefilterMap,
				const Ref<Texture2D>& _brdfLUT)
			:model(_model),envMap(_envMap),irradianceMap(_irradianceMap),
			prefilterMap(_prefilterMap),brdfLUT(_brdfLUT){}
		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent&) = default;
		SkyboxComponent(SkyboxComponent&&) = default;

	};
	struct PbrMeterialComponent
	{
		Vec3 albedo;
		float metallic;
		float roughness;
		float ao;
		Ref<Texture2D> albedoMap;
		Ref<Texture2D> metallicMap;
		Ref<Texture2D> roughnessMap;
		Ref<Texture2D> aoMap;
		PbrMeterialComponent() = default;
		PbrMeterialComponent(const PbrMeterialComponent&) = default;
		PbrMeterialComponent(PbrMeterialComponent&&) = default;
	};
	struct PerspectiveCameraComponent
	{
		bool isPrimary=false;
		PerspectiveCamera camera;
		PerspectiveCameraComponent() 
			:camera(Math::PI / 4, 0.1, 1000, 1){}
		PerspectiveCameraComponent(Real fovy,Real zNear,Real zFar,Real aspectRatio) 
			:camera(fovy,zNear,zFar,aspectRatio){}
		PerspectiveCameraComponent(const PerspectiveCameraComponent&)=default;
	};
	struct LuaScriptComponent
	{
		std::string script;
		std::string path;
		LuaScriptComponent() = default;
		LuaScriptComponent(const LuaScriptComponent&) = default;
		LuaScriptComponent(const std::string& _script, const std::string& _path)
			:script(_script),path(_path) {}
	};
	
}
