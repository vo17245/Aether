#include "PlaneSystem.h"
#include "Aether/Render.h"
#include "Aether/Render/ShaderSource.h"
#include "Resource/Shader.h"
namespace Aether
{
	namespace Editor
	{
		PlaneComponent::PlaneComponent()
			:m_Scaling(1.0),m_Size(500.f),m_LineWidth(0.1)
		{
			//model
			float pos[12] = {
				-1.0f,1.0f,0.0f,
				1.0f,1.0f,0.0f,
				1.0f,-1.0f,0.0f,
				-1.0f,-1.0f,0.0f
			};
			uint32_t index[6] = {
				0,1,2,
				2,3,0
			};
			auto modelRef = CreateRef<Model>();
			auto& model = *modelRef;
			auto positionBufferIndex = model.buffers.size();
			model.buffers.emplace_back(
				Buffer(
					(std::byte*)pos,
					((std::byte*)pos + sizeof(float) * 12)
				)
			);
			auto indexBufferIndex = model.buffers.size();
			model.buffers.emplace_back(
				Buffer(
					(std::byte*)index,
					(std::byte*)index + sizeof(uint32_t) * 6
				)
			);
			auto positionBufferViewIndex = model.bufferViews.size();
			model.bufferViews.emplace_back(
				BufferView(
					positionBufferIndex,//buffer
					0,//begin
					sizeof(float)*12,//end
					BufferView::Target::VERTEX_BUFFER,//target
					modelRef.get()//model
				)
			);
			auto positionAccessorIndex = model.accessors.size();
			model.accessors.emplace_back(
				Accessor(
					positionBufferIndex,
					sizeof(float)*3,
					ElementType::VEC3,
					0,//offset in bufferview,the first element begin
					modelRef.get()
				)
			);
			auto indexBufferViewIndex = model.bufferViews.size();
			model.bufferViews.emplace_back(
				BufferView(
					indexBufferIndex,//buffer
					0,//begin
					sizeof(unsigned int) * 6,//end
					BufferView::Target::INDEX_BUFFER,//target
					modelRef.get()//model
				)
			);
			auto indexAccessorIndex = model.accessors.size();
			model.accessors.emplace_back(
				Accessor(
					indexBufferViewIndex,
					sizeof(uint32_t),
					ElementType::UNSIGNED_INT32,
					0,
					modelRef.get()
				)
			);
			auto primitiveIndex = model.primitives.size();
			model.primitives.emplace_back(
				Primitive(modelRef.get())
			);
			model.primitives[primitiveIndex].AddAttribute(
				"POSITION", positionAccessorIndex
			);
			model.primitives[primitiveIndex].SetIndices(indexAccessorIndex);
			auto meshIndex = model.meshes.size();
			model.meshes.emplace_back(
				Mesh()
			);
			model.meshes[meshIndex].primitive = primitiveIndex;
			model.meshes[meshIndex].material = 0;
			m_XzPlaneQuad = modelRef;
			//shader
			auto shaderSrc=ShaderSource(std::string(aether_editor_plane_vs.begin(), aether_editor_plane_vs.end()), std::string(aether_editor_plane_fs.begin(), aether_editor_plane_fs.end()));
			m_Shader = Shader::Create(shaderSrc);
			AETHER_ASSERT(m_Shader&&"failed to create axis plane shader");
		}
		void PlaneSystem::OnRender(Scene& scene)
		{
			auto view = scene.GetAllEntitiesWith<PlaneComponent>();
			for (const auto& [entity, pc] : view.each())
			{
				if (!pc.m_XzPlaneQuad->hasBind)
				{
					pc.m_XzPlaneQuad->Bind();
				}
				// bind shader
				pc.m_Shader->Bind();
				// set uniform
				// render
				pc.m_XzPlaneQuad->Render();
			}

		}
	}
}