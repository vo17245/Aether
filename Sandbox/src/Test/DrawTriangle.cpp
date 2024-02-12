#include "DrawTriangle.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Render/IndexBuffer.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Aether/Resource/Model/BufferView.h"
#include "Aether/Resource/Model/Mesh.h"
#include "Aether/Resource/Model/Primitive.h"
#include "Test/DrawTriangle.h"
#include "Test/TestRegister.h"
#include "Aether/Core/Log.h"
#include "Aether/Resource/Model/ModelLoader.h"


namespace Aether
{
    namespace Test
    {
       
        REGISTER_TEST(DrawTriangle);
        DrawTriangle::DrawTriangle()
            :m_UseLowLevelApi(true)
        {
            //def data
            float position[] = {
            // Vertex 1
            -0.5f, -0.5f, 0.0f,
            // Vertex 2
            0.5f, -0.5f, 0.0f,
            // Vertex 3
            0.0f, 0.5f, 0.0f
            };
            // Normal array
            float normals[] = {
                // Normal 1
                0.0f, 0.0f, 1.0f,
                // Normal 2
                0.0f, 0.0f, 1.0f,
                // Normal 3
                0.0f, 0.0f, 1.0f
            };
            uint32_t indices[]={0,1,2};
            std::string vs=
            "#version 450 core\n"
            "layout(location = 0) in vec3 a_Position;\n"
            "layout(location=1) in vec3 a_Normal;\n"
            "out vec3 v_Normal;\n"
            "void main()\n"
            "{\n"
            "    v_Normal=a_Normal;\n"
            "    gl_Position=vec4(a_Position,1);\n"
            "}\n";
            std::string fs=
            "#version 450 core\n"
            "out vec4 color;\n"
            "in vec3 v_Normal;"
            "void main()\n"
            "{\n"
            "    color=vec4(1.0,0.0,0.0,1.0);\n"
            "}\n";
            //shader
            auto shaderSrc=ShaderSource(vs,fs);
            m_Shader=Shader::Create(shaderSrc);
            //low level api data
            m_VAO=VertexArray::Create();
            m_Pos=VertexBuffer::Create(position, sizeof(position));
            m_Normal=VertexBuffer::Create(normals, sizeof(normals));
            m_IBO=IndexBuffer::Create((std::byte*)indices,sizeof(indices));


            m_VAO->Bind();
            m_Pos->Bind();
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            m_Normal->Bind();
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(int), (void*)0);
            //model 
            Buffer buf_pos((std::byte*)position,(std::byte*)position+sizeof(position));
            Buffer buf_normal((std::byte*)normals,(std::byte*)normals+sizeof(normals));
            Buffer buf_indices((std::byte*)indices,(std::byte*)indices+sizeof(indices));
            m_Model=CreateRef<Model>();
            size_t pos_buf_index=m_Model->buffers.size();
            m_Model->buffers.emplace_back(std::move(buf_pos));
            size_t normal_buf_index=m_Model->buffers.size();
            m_Model->buffers.emplace_back(std::move(buf_normal));
            size_t indices_buf_index=m_Model->buffers.size();
            m_Model->buffers.emplace_back(std::move(buf_indices));
            size_t pos_buf_view_index=m_Model->bufferViews.size();
            m_Model->bufferViews.emplace_back(BufferView(pos_buf_index,0,sizeof(position),
            BufferView::Target::VERTEX_BUFFER,m_Model.get()));
            size_t normal_buf_view_index=m_Model->bufferViews.size();
            m_Model->bufferViews.emplace_back(BufferView(normal_buf_index,0,sizeof(normals),
            BufferView::Target::VERTEX_BUFFER,m_Model.get()));

            size_t indices_buf_view_index=m_Model->bufferViews.size();
            m_Model->bufferViews.emplace_back(BufferView(indices_buf_index,0,sizeof(indices),
            BufferView::Target::INDEX_BUFFER,m_Model.get()));
            size_t pos_accessor_index=m_Model->accessors.size();
            m_Model->accessors.emplace_back(Accessor(pos_buf_view_index,Accessor::GetElementSize(ElementType::VEC3),ElementType::VEC3,0,m_Model.get()));
            size_t normal_accessor_index=m_Model->accessors.size();
            m_Model->accessors.emplace_back(normal_buf_view_index,
            Accessor::GetElementSize(ElementType::VEC3),ElementType::VEC3,
            0,m_Model.get());
            size_t indices_accessor_index=m_Model->accessors.size();
            m_Model->accessors.emplace_back(indices_buf_view_index,
            Accessor::GetElementSize(ElementType::UNSIGNED_INT32),
            ElementType::UNSIGNED_INT32,0,m_Model.get());
            auto primitive=Primitive(m_Model.get());
            primitive.SetIndices(indices_accessor_index);
            primitive.AddAttribute("NORMAL", normal_accessor_index);
            primitive.AddAttribute("POSITION", pos_accessor_index);
            size_t primitive_index=m_Model->primitives.size();
            m_Model->primitives.emplace_back(primitive);
            auto mesh=Mesh{primitive_index,0};
            size_t mesh_index=m_Model->meshes.size();
            m_Model->meshes.emplace_back(mesh);
            auto node=Node();
            node.meshes.emplace_back(mesh_index);
            m_Model->nodes.emplace_back(node);
            m_Model->Bind();


            

        }
        DrawTriangle::~DrawTriangle()
        {

        }
        void DrawTriangle::OnRender()
        {
            if(m_UseLowLevelApi)
            {
                m_Shader->Bind();
                OpenGLApi::DrawElements(*m_VAO, *m_IBO, 3);
            }
            else 
            {
                m_Model->Render();
            }
            
        }
        void DrawTriangle::OnImGuiRender()
        {
            ImGui::Begin("DrawTriangle");
            ImGui::Checkbox("Use Low Level Api", &m_UseLowLevelApi);
            ImGui::End();
        }
    }
}