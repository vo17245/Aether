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
        m_VAO=VertexArray::Create();
        m_VAO->Bind();
        if(m_Indices)
        {
            Accessor& accessor=m_Model->accessors[m_Indices.value()];
            //indices accessor element type must be unsigned int32
            if(accessor.GetElementType()!=ElementType::UNSIGNED_INT32)
            {
                AETHER_ASSERT(false&&"indices accessor element type must be unsigned int32");
                AETHER_DEBUG_LOG("indices accessor element type not unsigned int32");
                return;
            }
            
            BufferView& bufferView=m_Model->bufferViews[accessor.GetBufferView()];
            Buffer& buffer=m_Model->buffers[bufferView.GetBuffer()];
            m_IBO=CreateRef<IndexBuffer>((uint32_t*)buffer.data(),buffer.size()/4);
        }
    }
    void Primitive::Unbind()
    {
        m_VAO.reset();
        m_IBO.reset();
    }
}