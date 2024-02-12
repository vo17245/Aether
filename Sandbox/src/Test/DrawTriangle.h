#pragma once
#include "Aether/Render/IndexBuffer.h"
#include "Test/Test.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Render/Shader.h"
#include "Aether/Render/VertexBuffer.h"
#include "Aether/Render/VertexArray.h"
namespace Aether
{
    namespace Test
	{
        class DrawTriangle:public Test
        {
        public:
            DrawTriangle();
            ~DrawTriangle();
            void OnRender()override;
            void OnImGuiRender()override;
        private:
            Ref<Shader> m_Shader;
            Ref<VertexBuffer> m_Pos;
            Ref<VertexBuffer> m_Normal;
            Ref<VertexArray> m_VAO;
            Ref<IndexBuffer> m_IBO;
            bool m_UseLowLevelApi;
            Ref<Model> m_Model;
        

        };
    }
}