#include "BufferView.h"
#include "Aether/Render/VertexBuffer.h"
#include "Model.h"
namespace Aether
{
    void BufferView::Bind()
    {
        m_VBO=VertexBuffer::Create(m_Model->buffers[m_Buffer].data()+m_Begin,m_End-m_Begin);
    }
    void BufferView::Unbind()
    {
        m_VBO.reset();
    }
}