#include "BufferView.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/IndexBuffer.h"
#include "Aether/Render/VertexBuffer.h"
#include "Model.h"
namespace Aether
{
    void BufferView::Bind()
    {
        if(!m_VBO && !m_IBO)
        {
            if(m_Target==Target::INDEX_BUFFER)
            {
                m_VBO=VertexBuffer::Create(m_Model->buffers[m_Buffer].data()+m_Begin,m_End-m_Begin);
            }
            else if(m_Target==Target::INDEX_BUFFER)
            {
                m_IBO=CreateRef<IndexBuffer>(m_Model->buffers[m_Buffer].data()+m_Begin,m_End-m_Begin);
            }
            else
            {
                AETHER_ASSERT(false&&"unknown BufferView Target");
            }
        }
        else 
        {
            if(m_Target==Target::INDEX_BUFFER)
            {
                m_VBO->Bind();
            }
            else if(m_Target==Target::INDEX_BUFFER)
            {
                m_IBO->Bind();
            }
            else
            {
                AETHER_ASSERT(false&&"unknown BufferView Target");
            }


        }
        
        
    }
    void BufferView::Unbind()
    {
        if(m_VBO)
            m_VBO.reset();
        if(m_IBO)
            m_IBO.reset();
    }
}