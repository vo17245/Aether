#include "SceneSerializer.h"
#include "SceneStorage.h"
#include <fstream>
#include "../Core/ResourceCache.h"
#include "../Utils/FileUtils.h"

namespace Aether
{
	bool SceneSerializer::WriteFile(Scene& scene, const std::string& name,
		const Camera& camera, const std::filesystem::path& path)
	{
		std::ofstream ofs(path);
		if (!ofs.is_open())
		{
			AETHER_DEBUG_LOG_ERROR("failed to open file {}", path.string());
			return false;
		}
		std::vector<std::byte> buf = WriteMem(scene, name, camera);
		ofs.write((const char*)buf.data(), buf.size());
		return true;
	}

	std::vector<std::byte> SceneSerializer::WriteMem(Scene& scene, const std::string& name,
		const Camera& camera)
	{
		std::vector<std::byte> buf;
		size_t sceneStorageHeaderOffset = 0;
		buf.resize(buf.size() + sizeof(SceneStorageHeader));
		((SceneStorageHeader*)(buf.data() + sceneStorageHeaderOffset))->camera.view = camera.GetView();
		((SceneStorageHeader*)(buf.data() + sceneStorageHeaderOffset))->camera.projection = camera.GetProjection();
		((SceneStorageHeader*)(buf.data() + sceneStorageHeaderOffset))->entityCnt = 0;
		((SceneStorageHeader*)(buf.data() + sceneStorageHeaderOffset))->nameLength = name.size();
		buf.insert(buf.end(), (std::byte*)name.data(), (std::byte*)name.data() + name.size());
		auto view = scene.GetAllEntitiesWith<IDComponent>();
		for (auto& [entity, ic] : view.each())
		{
			size_t entityStorageHeaderOffset = buf.size();
			buf.resize(buf.size() + sizeof(EntityStorageHeader));
			((SceneStorageHeader*)(buf.data() + sceneStorageHeaderOffset))->entityCnt++;
			Entity e = { entity,&scene };

			((EntityStorageHeader*)(buf.data() + entityStorageHeaderOffset))->componentCnt = 0;
			//IDComponent
			((EntityStorageHeader*)(buf.data() + entityStorageHeaderOffset))->componentCnt++;
			size_t componentStorageHeaderOffset = buf.size();
			buf.resize(buf.size() + sizeof(ComponentStorageHeader));

			((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->type = 0;
			((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->bufferLength = sizeof(UUIDStorage);
			buf.insert(buf.end(), (std::byte*)&ic, (std::byte*)&ic + sizeof(UUIDStorage));


			if (e.HasComponent<TagComponent>())
			{
				((EntityStorageHeader*)(buf.data() + entityStorageHeaderOffset))->componentCnt++;
				size_t componentStorageHeaderOffset = buf.size();
				buf.resize(buf.size() + sizeof(ComponentStorageHeader));
				auto& tc = e.GetComponent<TagComponent>();
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->type = 1;
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->bufferLength = tc.tag.size();
				buf.insert(buf.end(), (std::byte*)tc.tag.data(), (std::byte*)tc.tag.data() + tc.tag.size());
			}

			if (e.HasComponent<TransformComponent>())
			{
				((EntityStorageHeader*)(buf.data() + entityStorageHeaderOffset))->componentCnt++;
				size_t componentStorageHeaderOffset = buf.size();
				buf.resize(buf.size() + sizeof(ComponentStorageHeader));
				auto& tc = e.GetComponent<TransformComponent>();
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->type = 2;
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->bufferLength = sizeof(TransformStorage);
				
				size_t transformStorageOffset = buf.size();
				buf.resize(buf.size() + sizeof(TransformStorage));
				((TransformStorage*)(buf.data() + transformStorageOffset))->position = tc.position;
				((TransformStorage*)(buf.data() + transformStorageOffset))->rotation = tc.rotation;
				((TransformStorage*)(buf.data() + transformStorageOffset))->scaling = tc.scaling;
			}
			if (e.HasComponent<VisualComponent>())
			{
				((EntityStorageHeader*)(buf.data() + entityStorageHeaderOffset))->componentCnt++;
				size_t componentStorageHeaderOffset = buf.size();
				buf.resize(buf.size() + sizeof(ComponentStorageHeader));
				auto& vc = e.GetComponent<VisualComponent>();
				std::string path = vc.model->GetPath().string();
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->type = 3;
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->bufferLength = path.size();
				buf.insert(buf.end(), (std::byte*)path.data(), (std::byte*)path.data() + path.size());
			}
			if (e.HasComponent<PointLightComponent>())
			{
				((EntityStorageHeader*)(buf.data() + entityStorageHeaderOffset))->componentCnt++;
				size_t componentStorageHeaderOffset = buf.size();
				buf.resize(buf.size() + sizeof(ComponentStorageHeader));
				auto& plc = e.GetComponent<PointLightComponent>();
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->type = 4;
				((ComponentStorageHeader*)(buf.data() + componentStorageHeaderOffset))->bufferLength = sizeof(PointLightStorage);
				size_t pointLightStorageOffset = buf.size();
				buf.resize(buf.size() + sizeof(PointLightStorage));
				((PointLightStorage*)(buf.data() + pointLightStorageOffset))->color = plc.light.GetColor();
				((PointLightStorage*)(buf.data() + pointLightStorageOffset))->position = plc.light.GetPosition();

			}
		}
		return buf;

	}
	
	std::optional<DeserializationResult> SceneSerializer::LoadFile(const std::filesystem::path& path)
	{
		auto res=FileUtils::ReadFile(path);
		if (!res)
		{
			return std::nullopt;
		}
		return LoadMem(res.value());
	}
	std::optional<DeserializationResult> SceneSerializer::LoadMem(const std::vector<std::byte>& buf)
	{
		DeserializationResult res;
		res.scene = CreateRef<Scene>();
		size_t offset = 0;
		SceneStorageHeader& sceneStorageHeader = *(SceneStorageHeader*)buf.data();
		offset += sizeof(SceneStorageHeader);
		res.camera.GetProjection() = sceneStorageHeader.camera.projection;
		res.camera.GetView() = sceneStorageHeader.camera.view;
		res.name.insert(res.name.end(), (const char*)buf.data(), (const char*)buf.data() + sceneStorageHeader.nameLength);
		offset += sceneStorageHeader.nameLength;
		for (size_t i = 0;i < sceneStorageHeader.entityCnt;i++)
		{
			entt::entity entity=res.scene->m_Registry.create();
			Entity e = { entity,res.scene.get() };
			EntityStorageHeader& entityStorageHeader = *(EntityStorageHeader*)(buf.data() + offset);
			offset += sizeof(EntityStorageHeader);
			for (size_t j = 0;j < entityStorageHeader.componentCnt;j++)
			{
				ComponentStorageHeader& componentStorageHeader= 
					*(ComponentStorageHeader*)(buf.data() + offset);
				offset += sizeof(ComponentStorageHeader);
				switch (componentStorageHeader.type)
				{
				case 0://IDComponent
				{
					UUIDStorage& uuidStorage = *(UUIDStorage*)(buf.data() + offset);
					offset += sizeof(UUIDStorage);
					e.AddComponent<IDComponent>(uuidStorage.uuid);
					break;
				}
				case 1://TagComponent
				{
					std::string tag;
					tag.insert(tag.end(), (const char*)buf.data() + offset,
						(const char*)buf.data() + offset + componentStorageHeader.bufferLength);
					offset += componentStorageHeader.bufferLength;
					e.AddComponent<TagComponent>(tag);
					break;
				}
				case 2://TransformComponent
				{
					TransformStorage& transformStorage = *(TransformStorage*)(buf.data() + offset);
					offset += sizeof(TransformStorage);
					auto& tc = e.AddComponent<TransformComponent>();
					tc.position = transformStorage.position;
					tc.rotation = transformStorage.rotation;
					tc.scaling = transformStorage.scaling;
					break;
				}
					
				case 3://VisualComponent
				{
					std::string modelPath;
					modelPath.insert(modelPath.end(), (const char*)buf.data() + offset,
						(const char*)buf.data() + offset + componentStorageHeader.bufferLength);
					offset += sizeof(UUIDStorage);
					auto cacheRes = FileCache<ModelAsset>::Get(modelPath);
					if (!cacheRes)
					{
						AETHER_DEBUG_LOG_ERROR("failed to load model,model path: {}", modelPath);
						return std::nullopt;
					}
					Ref<Model> model = CreateRef<Model>(cacheRes.value());
					e.AddComponent<VisualComponent>(model);
					break;
				}
				case 4://PointLightComponent
				{
					PointLightStorage& pointLightStorage = *(PointLightStorage*)(buf.data() + offset);
					offset += sizeof(PointLightStorage);
					e.AddComponent<PointLightComponent>(pointLightStorage.color,
						pointLightStorage.position);
					break;
				}
					
				default:
				{
					AETHER_DEBUG_LOG_ERROR("unknown component type {}", componentStorageHeader.type);
					return std::nullopt;
				}
					
				}

			}
		}
		return res;
	}
}
