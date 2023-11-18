/*
* **********************************************
* .aetherscene File Format Specification
* **********************************************
* CameraStorage
* {
*	float view[16];
*	float projection[16];
* }
* EntityStorage
* {
*	unsigned int componentCnt;
*	ComponentStorage components[componentCnt];
* }
* SceneStorage
* {
*	unsigned int nameLength;
*	unsigned int entityCnt;
*   CameraStorage camera;
*	char name[nameLength];
*	EntityStorage entities[entityCnt];
*
* }
* **********************************************
* ComponentStorage Specification
* **********************************************
* ComponentStorage
* {
*	unsigned int type;
*	unsigned int bufferLength;
*	char buffer[bufferLength];
* }
* type==0
* buffer layout:
* IDComponentStorage
* {
*	uint64_t id
* }
* type==1
* buffer layout:
* TagComponentStorage
* {
*	char tag[bufferLength];
* }
* type==2
* buffer layout:
* TransformComponent
* {
*	float position[3];
*	float rotation[3];
*	float scaling[3];
* }
* type==3
* buffer layout:
* VisualComponent
* {
*	char modelPath[bufferLength];
* }
* type==4
* buffer layout
* PointLightComponent
* {
*	float color[3];
*	float position[3];
* }
* 
* 
*/
#pragma once
#include <stdint.h>
#include "../Core/Math.h"
#include "Component.h"
#include <vector>
#include "../Render/Camera.h"
namespace Aether
{
#pragma pack(push,1)
	
	struct EntityStorageHeader
	{
		uint32_t componentCnt;
	};
	struct CameraStorage
	{
		Mat4 view;
		Mat4 projection;
	};
	struct SceneStorageHeader
	{
		uint32_t nameLength;
		uint32_t entityCnt;
		CameraStorage camera;
	};
	struct ComponentStorageHeader
	{
		uint32_t type;
		uint32_t bufferLength;
		ComponentStorageHeader(uint32_t _type, uint32_t _bufferLength)
			:type(_type), bufferLength(_bufferLength) {}
	};
	struct PointLightStorage
	{
		Vec3 color;
		Vec3 position;
	};
	struct TransformStorage
	{
		Vec3 position;
		Vec3 rotation;
		Vec3 scaling;
	};
	struct UUIDStorage
	{
		uint64_t uuid;
	};
#pragma pack(pop)
}