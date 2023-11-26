#include "SceneSerializer.h"
#include <fstream>
#include <optional>
#include "../Utils/FileUtils.h"
#include "nlohmann/json.hpp"
#include "../Core/Log.h"
#include "../Asset/ModelAssetImporter.h"

using json = nlohmann::json;
namespace Aether
{
    static json Vec32Json(const Vec3 v)
    {
        json j = json::array();
        j.push_back(v.x());
        j.push_back(v.y());
        j.push_back(v.z());
        return j;
    }
    static std::optional<json> CameraStorage2Json(const CameraStorage & camera)
    {
        json json_camera = json::object();
        if (camera.type == CameraStorage::CameraType::Perspective)
        {
            if (!camera.perspectiveCamera)
            {
                AETHER_DEBUG_LOG_ERROR("camera.perspectiveCamera is empty");
                return std::nullopt;
            }
            auto& pc = camera.perspectiveCamera.value();
            json_camera["type"] = "perspective";

            json_camera["fovy"] = pc.GetFovy();
            json_camera["zNear"] = pc.GetZNear();
            json_camera["zFar"] = pc.GetZFar();
            json_camera["aspectRatio"] = pc.GetAspectRatio();
            json_camera["position"] = Vec32Json(pc.GetPosition());
            json_camera["rotation"] = Vec32Json(pc.GetRotation());
        }
        else if (camera.type == CameraStorage::CameraType::Orthographic)
        {
            if (!camera.orthographicCamera)
            {
                AETHER_DEBUG_LOG_ERROR("camera.orthographicCamera is empty");
                return std::nullopt;
            }
            auto& oc = camera.orthographicCamera.value();
            json_camera["type"] = "orthographic";
            json_camera["position"] = Vec32Json(oc.GetPosition());
            json_camera["rotation"] = Vec32Json(oc.GetRotation());
            json_camera["left"] = oc.GetLeft();
            json_camera["right"] = oc.GetRight();
            json_camera["buttom"] = oc.GetButtom();
            json_camera["top"] = oc.GetTop();
            json_camera["near"] = oc.GetNear();
            json_camera["far"] = oc.GetFar();
        }
        else
        {
            AETHER_DEBUG_LOG_ERROR("unknown cameratype {}", uint8_t(camera.type));
            return std::nullopt;
        }
        return json_camera;
    }
    static std::optional<json> Scene2Json(Scene& scene)
    {
        json json_scene = json::object();
        json_scene["entities"] = json::array();
        auto view = scene.GetAllEntitiesWith<IDComponent>();
        for (const auto& [entity, ic] : view.each())
        {
            Entity e = { entity,&scene };
            json json_entity = json::object();
            //IDComponent
            json json_IDComponent = json::object();
            json_IDComponent["id"] =uint64_t(ic.id);
            json_entity["IDComponent"] =json_IDComponent ;
            //TagComponent
            if (e.HasComponent<TagComponent>())
            {
                auto& tc=e.GetComponent<TagComponent>();
                json json_TagComponent = json::object();
                json_TagComponent["tag"] = tc.tag;
                json_entity["TagComponent"] = json_TagComponent;
            }
            //VisualComponent
            if (e.HasComponent<VisualComponent>())
            {
                auto& vc = e.GetComponent<VisualComponent>();
                json json_VisualComponent = json::object();
                json_VisualComponent["modelPath"] = vc.model->GetPath().string();
                json_entity["VisualComponent"] = json_VisualComponent;
            }
            //PointLightComponent
            if (e.HasComponent<PointLightComponent>())
            {
                auto& plc = e.GetComponent<PointLightComponent>();
                json json_PointLightComponent = json::object();
                json_PointLightComponent["position"] = Vec32Json(plc.light.GetPosition());
                json_PointLightComponent["color"] = Vec32Json(plc.light.GetColor());
                json_entity["PointLightComponent"] = json_PointLightComponent;
            }
            //TransformComponent
            if (e.HasComponent<TransformComponent>())
            {
                auto& tc = e.GetComponent<TransformComponent>();
                json json_TransformComponent = json::object();
                json_TransformComponent["position"] = Vec32Json(tc.position);
                json_TransformComponent["rotation"] = Vec32Json(tc.rotation);
                json_TransformComponent["scaling"] = Vec32Json(tc.scaling);
                json_entity["TransformComponent"] = json_TransformComponent;
            }
            json_scene["entities"].push_back(json_entity);
        }
        return json_scene;
    }
    bool SceneSerializer::WriteFile(Scene& scene, const std::string& name, const CameraStorage& camera, const std::filesystem::path& path)
    {
        std::ofstream ofs(path,std::ios_base::binary|std::ios_base::out);
        if (!ofs.is_open())
        {
            return false;
        }
        auto res=WriteMem(scene, name, camera);
        if (!res)
        {
            return false;
        }
        ofs.write(res.value().data(), res.value().size());
        return true;
    }
    std::optional<std::string> SceneSerializer::WriteMem(Scene& scene, const std::string& name, const CameraStorage& camera)
    {
        json file = json::object();
        file["name"] = name;
        auto cameraRes = CameraStorage2Json(camera);
        if (!cameraRes)
        {
            AETHER_DEBUG_LOG_ERROR("failed to encode camera");
            return std::nullopt;
        }
        file["camera"] = std::move(cameraRes.value());
        file["name"] = name;
        auto sceneRes = Scene2Json(scene);
        if (!sceneRes)
        {
            AETHER_DEBUG_LOG_ERROR("failed to encode scene");
            return std::nullopt;
        }
        file["scene"] = sceneRes.value();
        return file.dump(4);
    }
    std::optional<DeserializationResult> SceneSerializer::LoadFile(const std::filesystem::path& path)
    {
        auto opt_buf = FileUtils::ReadFile(path);
        if (!opt_buf)
        {
            AETHER_DEBUG_LOG_ERROR("failed to load file {}", path.string());
            return std::nullopt;
        }
        return LoadMem(opt_buf.value());
    }
    bool CheckNumberArray(const json& json_array,size_t len)
    {
        if(!json_array.is_array())
        {
            return false;
        }
        if(!(json_array.size()==len))
        {
            return false;
        }
        for(size_t i=0;i<json_array.size();i++)
        {
            if(!json_array[i].is_number())
            {
                return false;
            }
        }
        return true;
    }
    Vec3 JsonNumberArray2Vec3(const json& json_array)
    {
        return Vec3(json_array[0],json_array[1],json_array[2]);
    }
    std::optional<json> GetFromObject(const json& json_obj,const std::string& key)
    {
        auto iter=json_obj.find(key);
        if(iter==json_obj.end())
        {
            return std::nullopt;
        }
        else {
            return *iter;
        }
    }
    static std::optional<CameraStorage> Json2CameraStorage(const json& json_cameraStorage)
    {
        if(!json_cameraStorage.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("not obj");
            return std::nullopt;
        }
        auto opt_json_aspectRatio=GetFromObject(json_cameraStorage,"aspectRatio");
        if(!opt_json_aspectRatio)
        {
            AETHER_DEBUG_LOG_ERROR("no valid aspectRatio");
            return std::nullopt;
        }
        const json& json_aspectRatio=opt_json_aspectRatio.value();
        if(!json_aspectRatio.is_number())
        {
            AETHER_DEBUG_LOG_ERROR("no valid aspectRatio");
            return std::nullopt;
        }
        float aspectRatio=json_aspectRatio;
        auto opt_json_fovy=GetFromObject(json_cameraStorage,"fovy");
        if(!opt_json_fovy)
        {
            AETHER_DEBUG_LOG_ERROR("no valid fovy");
            return std::nullopt;
        }
        const json& json_fovy=opt_json_fovy.value();
        
        if(!json_fovy.is_number())
        {
            AETHER_DEBUG_LOG_ERROR("no valid fovy");
            return std::nullopt;
        }
        float fovy=json_fovy;
        auto opt_json_position=GetFromObject(json_cameraStorage,"position");
        if(!opt_json_position)
        {
            AETHER_DEBUG_LOG_ERROR("no valid position");
            return std::nullopt;
        }
        const json& json_position=opt_json_position.value();
        if(!CheckNumberArray(json_position, 3))
        {
            AETHER_DEBUG_LOG_ERROR("no valid position");
            return std::nullopt;
        }
        Vec3 position=JsonNumberArray2Vec3(json_position);
        auto opt_json_rotation=GetFromObject(json_cameraStorage,"rotation");
        if(!opt_json_rotation)
        {
            AETHER_DEBUG_LOG_ERROR("no valid rotation");
            return std::nullopt;
        }
        const json& json_rotation=opt_json_rotation.value();
        if(!CheckNumberArray(json_rotation, 3))
        {
            AETHER_DEBUG_LOG_ERROR("no valid rotation");
            return std::nullopt;
        }
        Vec3 rotation=JsonNumberArray2Vec3(json_rotation);
        auto opt_json_type=GetFromObject(json_cameraStorage,"type");
        if(!opt_json_type)
        {
            AETHER_DEBUG_LOG_ERROR("no valid type");
            return std::nullopt;
        }

        json json_type=opt_json_type.value();
        if(!json_type.is_string())
        {
            AETHER_DEBUG_LOG_ERROR("no valid perspective");
            return std::nullopt;
        }
        std::string type=json_type;
        auto opt_json_zFar=GetFromObject(json_cameraStorage, "zFar");
        if(!opt_json_zFar)
        {
            AETHER_DEBUG_LOG_ERROR("no valid zFar");
            return std::nullopt;
        }
        const json& json_zFar=opt_json_zFar.value();
        if(!json_zFar.is_number())
        {
            AETHER_DEBUG_LOG_ERROR("no valid zFar");
            return std::nullopt;
        }
        float zFar=json_zFar;
        auto opt_json_zNear=GetFromObject(json_cameraStorage, "zNear");
        if(!opt_json_zNear)
        {
            AETHER_DEBUG_LOG_ERROR("no valid zNear");
            return std::nullopt;
        }
        const json& json_zNear=opt_json_zNear.value();
        if(!json_zNear.is_number())
        {
            AETHER_DEBUG_LOG_ERROR("no valid zNear");
            return std::nullopt;
        }
        float zNear=json_zNear;
        if(type.compare("perspective")==0)
        {
            PerspectiveCamera camera(fovy,zNear,zFar,aspectRatio);
            camera.GetPosition()=position;
            camera.GetRotation()=rotation;
            CameraStorage cameraStorage(camera);
            return cameraStorage;
        }
        else
        {
            AETHER_DEBUG_LOG_ERROR("unknown camera type {}",type);
            return std::nullopt;
        }
    }
    std::optional<IDComponent> Json2IDComponent(const json& json_ic)
    {
        if(!json_ic.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("invalid IDComponent");
            return std::nullopt;
        }
        auto opt_json_id=GetFromObject(json_ic, "id");
        if(!opt_json_id)
        {
            AETHER_DEBUG_LOG_ERROR("invalid IDComponent");
            return std::nullopt;
        }
        const json& json_id=opt_json_id.value();
        size_t id=json_id;
        IDComponent ic(UUID((uint64_t)id));
        return ic;
    }
    std::optional<TagComponent> Json2TagComponent(const json& json_tc)
    {
        if(!json_tc.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("invalid TagComponent");
            return std::nullopt;
        }
        auto opt_json_tag=GetFromObject(json_tc, "tag");
        if(!opt_json_tag)
        {
            AETHER_DEBUG_LOG_ERROR("invalid TagComponent");
            return std::nullopt;
        }
        if(!opt_json_tag->is_string())
        {
            AETHER_DEBUG_LOG_ERROR("invalid TagComponent");
            return std::nullopt;
        }
        std::string tag=opt_json_tag.value();
        return TagComponent(tag);
    }
    std::optional<TransformComponent> Json2TransformComponent(const json& json_TransformComponent)
    {
        if(!json_TransformComponent.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        auto opt_json_position=GetFromObject(json_TransformComponent, "position");
        if(!opt_json_position)
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        const json& json_position=opt_json_position.value();
        if(!CheckNumberArray(json_position, 3))
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        Vec3 position=JsonNumberArray2Vec3(json_position);

        auto opt_json_rotation=GetFromObject(json_TransformComponent, "rotation");
        if(!opt_json_rotation)
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        const json& json_rotation=opt_json_rotation.value();
        if(!CheckNumberArray(json_rotation, 3))
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        Vec3 rotation=JsonNumberArray2Vec3(json_rotation);

        auto opt_json_scaling=GetFromObject(json_TransformComponent, "scaling");
        if(!opt_json_scaling)
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        const json& json_scaling=opt_json_scaling.value();
        if(!CheckNumberArray(json_scaling, 3))
        {
            AETHER_DEBUG_LOG_ERROR("invalid TransformComponent");
            return std::nullopt;
        }
        Vec3 scaling=JsonNumberArray2Vec3(json_scaling);
        TransformComponent transformComponent;
        transformComponent.rotation=rotation;
        transformComponent.position=position;
        transformComponent.scaling=scaling;
        return transformComponent;
    }
    std::optional<VisualComponent> Json2VisualComponent(const json& json_vc)
    {
        if(!json_vc.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("invalid VisualComponent");
            return std::nullopt;
        }
        auto opt_json_modelPath=GetFromObject(json_vc, "modelPath");
        if(!opt_json_modelPath)
        {
            AETHER_DEBUG_LOG_ERROR("invalid VisualComponent");
            return std::nullopt;
        }
        std::string modelPath=opt_json_modelPath.value();
        auto opt_modelAsset = ModelAssetImporter::LoadFromFile(modelPath);
        if(!opt_modelAsset)
        {
            AETHER_DEBUG_LOG_ERROR("failed to load model from {}",modelPath);
            return std::nullopt;
        }
        auto model = CreateRef<Model>(opt_modelAsset.value());
        return VisualComponent(model);
    }
    std::optional<PointLightComponent> Json2PointLightComponent(const json& json_plc)
    {
        if(!json_plc.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("invalid PointLightComponent");
            return std::nullopt;
        }
        auto opt_json_color=GetFromObject(json_plc, "color");
        if(!opt_json_color)
        {
            AETHER_DEBUG_LOG_ERROR("invalid PointLightComponent");
            return std::nullopt;
        }
        const json& json_color=opt_json_color.value();
        if(!CheckNumberArray(json_color, 3))
        {
            AETHER_DEBUG_LOG_ERROR("invalid PointLightComponent");
            return std::nullopt;
        }
        Vec3 color=JsonNumberArray2Vec3(json_color);

        auto opt_json_position=GetFromObject(json_plc, "position");
        if(!opt_json_position)
        {
            AETHER_DEBUG_LOG_ERROR("invalid PointLightComponent");
            return std::nullopt;
        }
        const json& json_position=opt_json_position.value();
        if(!CheckNumberArray(json_position, 3))
        {
            AETHER_DEBUG_LOG_ERROR("invalid PointLightComponent");
            return std::nullopt;
        }
        Vec3 position=JsonNumberArray2Vec3(json_position);
        return PointLightComponent(color,position);

    }
    std::optional<Ref<Scene>> Json2Scene(const json& json_scene)
    {
        Ref<Scene> scene=CreateRef<Scene>();
        if(!json_scene.is_object())
        {
            AETHER_DEBUG_LOG_ERROR("json_scene is not obj");
            return std::nullopt;
        }
        auto opt_json_entities=GetFromObject(json_scene, "entities");
        if(!opt_json_entities)
        {
            AETHER_DEBUG_LOG_ERROR("no valid entities");
            return std::nullopt;
        }
        const json& json_entities=opt_json_entities.value();
        if(!json_entities.is_array())
        {
            AETHER_DEBUG_LOG_ERROR("no valid entities");
            return std::nullopt;
        }
        for(size_t i=0;i<json_entities.size();i++)
        {
            const Entity& const_entity=scene->CreateEntity();
            Entity& entity=(Entity&)const_entity;
            const json& json_entity=json_entities[i];
            if(!json_entity.is_object())
            {
                AETHER_DEBUG_LOG_ERROR("json_entity is not obj");
                return std::nullopt;
            }

            auto opt_json_IDComponent=GetFromObject(json_entity, "IDComponent");
            if(!opt_json_IDComponent)
            {
                AETHER_DEBUG_LOG_ERROR("entity must have IDComponent");
                return std::nullopt;
            }
            const json& json_IDComponent=opt_json_IDComponent.value();
            if(!json_IDComponent.is_object())
            {
                AETHER_DEBUG_LOG_ERROR("invalid IDComponent");
                return std::nullopt;
            }
            auto opt_ic=Json2IDComponent(json_IDComponent);
            if(!opt_ic)
            {
                AETHER_DEBUG_LOG_ERROR("failed to parse IDComponent");
                return std::nullopt;
            }
            const IDComponent& ic=opt_ic.value();
            //replace the IDComponent added when entity created
            entity.AddOrReplaceComponent<IDComponent>(ic);
            auto opt_json_TagComponent=GetFromObject(json_entity, "TagComponent");
            if(opt_json_TagComponent)
            {
                const json& json_TagComponent=opt_json_TagComponent.value();
                auto opt_tc=Json2TagComponent(json_TagComponent);
                if(!opt_tc)
                {
                    AETHER_DEBUG_LOG_ERROR("failed to parse TagComponent");
                    return std::nullopt;
                }
                entity.AddComponent<TagComponent>(std::move(opt_tc.value()));
            }
            auto opt_json_TransformComponent=GetFromObject(json_entity, "TransformComponent");
            if(opt_json_TransformComponent)
            {
                const json& json_TransformComponent=opt_json_TransformComponent.value();
                auto opt_TransformComponent=Json2TransformComponent(json_TransformComponent);
                if(!opt_TransformComponent)
                {
                    AETHER_DEBUG_LOG_ERROR("failed to parse TrasnformComponent");
                    return std::nullopt;
                }
                entity.AddComponent<TransformComponent>(std::move(opt_TransformComponent.value()));
            }
            auto opt_json_VisualComponent=GetFromObject(json_entity, "VisualComponent");
            if(opt_json_VisualComponent)
            {
                const json& json_VisualComponent=opt_json_VisualComponent.value();
                auto opt_vc=Json2VisualComponent(json_VisualComponent);
                if(!opt_vc)
                {
                    AETHER_DEBUG_LOG_ERROR("failed to parse VisualComponent");
                    return std::nullopt;
                }
                entity.AddComponent<VisualComponent>(opt_vc.value());
            }
            auto opt_json_PointLightComponent=GetFromObject(json_entity, "PointLightComponent");
            if(opt_json_PointLightComponent)
            {
                const json& json_plc=opt_json_PointLightComponent.value();
                auto opt_plc=Json2PointLightComponent(json_plc);
                if(!opt_plc)
                {
                    AETHER_DEBUG_LOG_ERROR("failed to parse PointLightComponent");
                    return std::nullopt;
                }
                entity.AddComponent<PointLightComponent>(std::move(opt_plc.value()));
            }
            
        } 
        return scene;
    }
    std::optional<DeserializationResult> SceneSerializer::LoadMem(const std::vector<std::byte>& buf)
    {
        
        json json_file = json::parse(buf);
        auto opt_json_cameraStorage=GetFromObject(json_file, "camera");
        if(!opt_json_cameraStorage)
        {
            AETHER_DEBUG_LOG_ERROR("no valid camera");
            return std::nullopt;
        }
        const json& json_cameraStorage=opt_json_cameraStorage.value();
        auto opt_cameraStorage=Json2CameraStorage(json_cameraStorage);
        if(!opt_cameraStorage)
        {
            AETHER_DEBUG_LOG_ERROR("failed to parse camera");
            return std::nullopt;
        }
        auto opt_json_name=GetFromObject(json_file, "name");
        if(!opt_json_name)
        {
            AETHER_DEBUG_LOG_ERROR("no valid name");
            return std::nullopt;
        }
        std::string name=opt_json_name.value();
        auto opt_json_scene=GetFromObject(json_file, "scene");
        if(!opt_json_scene)
        {
            AETHER_DEBUG_LOG_ERROR("no valid scene");
            return std::nullopt;
        }
        const json& json_scene=opt_json_scene.value();
        std::optional<Ref<Scene>> opt_scene=Json2Scene(json_scene);
        if(!opt_scene)
        {
            AETHER_DEBUG_LOG_ERROR("failed to parse scene");
            return std::nullopt;
        }
        DeserializationResult result(std::move(opt_scene.value()),std::move(opt_cameraStorage.value()),
        std::move(name));
        return result;
    }
}
