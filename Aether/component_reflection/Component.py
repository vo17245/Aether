filename="Component.h"

includes="""
#include "../Core/UUID.h"
#include "Aether/Core/Math.h"
#include "Aether/Render/Texture2D.h"
#include "Eigen/Core"
#include <vector>
#include "../Render/Light.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Render/CubeMap.h"
#include "Aether/Render/Camera.h"
#include "../Core/Config.h"
#include <filesystem>
#include "../Render/Transform.h"
"""#optional

structs=[
    {
        "name":"::Aether::IDComponent",
        "fields":[
            {
                "name":"id",
                "type":"UUID",
            }
            
        ],
        "constructors":[
            """
            IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& _id):id(_id){}
            """,
        ]
    },
    {
        "name":"::Aether::TagComponent",
        "fields":[
            {
                "name":"tag",
                "type":"std::string",
            }
            
        ],
        "constructors":[
            """
           TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& _tag)
			:tag(_tag) {}
            """,
        ]
    },
    {
        "name":"::Aether::TransformComponent",
        "fields":[
            {
                "name":"position",
                "type":"::Eigen::Vector3f",
            },
            {
                "name":"rotation",
                "type":"::Eigen::Vector3f",
                "comment":"旋转顺序为x y z",
            },
            {
                "type":"::Eigen::Vector3f", 
                "name":"scaling",
            },
            {
                "type":"Eigen::Matrix4f", 
                "name":"matrix",
            },
            


            
            
        ],
        "constructors":[
            """
          TransformComponent(const TransformComponent&) = default;
		TransformComponent()
			:position(Eigen::Vector3f::Zero()),
			rotation(Eigen::Vector3f::Zero()),
			scaling(1,1,1) ,matrix(Eigen::Matrix4f::Identity()){}
		void CalculateMatrix()
        {
        matrix = Transform::Translation(position)*Transform::Rotation(rotation)*Transform::Scale(scaling);
        }
            """,
        ]
    },
    {
        "name":"::Aether::PointLightComponent",
        "fields":[
            {
                "name":"light",
                "type":"PointLight",
            }
            
        ],
        "constructors":[
            """
            PointLightComponent(PointLightComponent&&)=default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent() = default;
		PointLightComponent(const ::Eigen::Vector3f& color, const ::Eigen::Vector3f& pos)
			:light(color,pos)
		{}
		PointLightComponent& operator=(const PointLightComponent& plc)
		{
			light=plc.light;
			return *this;
		}
            """,
        ]
    },
    {
        "name":"::Aether::MeshComponent",
        "fields":[
            {
                "name":"model",
                "type":"Ref<Model>",
            },
            {
                "name":"filePath",
                "type":"std::optional<std::string>",
            }
            
        ],
        "constructors":[
            """
            MeshComponent(const MeshComponent&) = default;
            """,
        ]
    },
    {
        "name":"::Aether::SkyboxComponent",
        "fields":[
            {
                "type":"Ref<Model>",
                "name":"model"
            },
		    {
                "type":"Ref<CubeMap>", 
                "name":"envMap"
            },
		    {
                "type":"Ref<CubeMap>",
                "name":"irradianceMap"
            },
            {
                "type":"Ref<CubeMap>",
                "name":"prefilterMap"
            },
		    {
                "type":"Ref<Texture2D>",
                "name":"brdfLUT"
            }
        ],
        "constructors":[
            """
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
            """,
        ]
    },
    {
        "name":"::Aether::PbrMeterialComponent",
        "fields":[
            {"type":"::Eigen::Vector3f",          "name":"albedo"},
		    {"type":"float",          "name":"metallic"},
		    {"type":"float",          "name":"roughness"},
		    {"type":"float",          "name":"ao"},
		    {"type":"Ref<Texture2D>", "name":"albedoMap"},
		    {"type":"Ref<Texture2D>", "name":"metallicMap"},
		    {"type":"Ref<Texture2D>", "name":"roughnessMap"},
		    {"type":"Ref<Texture2D>", "name":"aoMap"},
        ],
        "constructors":[
            """
           PbrMeterialComponent() = default;
		PbrMeterialComponent(const PbrMeterialComponent&) = default;
		PbrMeterialComponent(PbrMeterialComponent&&) = default;
            """,
        ]
    },
    {
        "name":"::Aether::PerspectiveCameraComponent",
        "fields":[
            {"type":"bool",          "name":"isPrimary"},
            {"type":"PerspectiveCamera","name":"camera" }
        ],
        "constructors":[
            """
           PerspectiveCameraComponent() 
			:camera(Math::PI / 4, 0.1, 1000, 1){}
		PerspectiveCameraComponent(Real fovy,Real zNear,Real zFar,Real aspectRatio) 
			:camera(fovy,zNear,zFar,aspectRatio){}
		PerspectiveCameraComponent(const PerspectiveCameraComponent&)=default;
            """,
        ]
    },
    {
        "name":"::Aether::LuaScriptComponent",
        "fields":[
            {"type":"std::string","name":"script" },
		    {"type":"std::string","name":"path"}
        ],
        "constructors":[
            """
            LuaScriptComponent() = default;
		LuaScriptComponent(const LuaScriptComponent&) = default;
		LuaScriptComponent(const std::string& _script, const std::string& _path)
			:script(_script),path(_path) {}
            """
        ]
    }


]#optional