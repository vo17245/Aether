#include "Primitive.h"
#include "Aether/Core/Log.h"
#include "Aether/Render/IndexBuffer.h"
#include "Aether/Render/VertexArray.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Aether/Resource/Model/Buffer.h"
#include "Model.h"
namespace Aether
{
    void Primitive::Bind()
    {
        if(!m_VAO)
        {
            m_VAO=VertexArray::Create();
        }
        m_VAO->Bind();
        
    }
    void Primitive::Unbind()
    {
        m_VAO.reset();
    }
    Primitive::~Primitive()
    {
        if(m_VAO)
            m_VAO.reset();
    }
    
}