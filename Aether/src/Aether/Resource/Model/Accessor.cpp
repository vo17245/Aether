#include "Accessor.h"
#include "Model.h"
namespace Aether
{
    AccessorIterator::AccessorIterator(size_t pos,size_t stride,size_t bufferView,
    size_t offset,Model* model)
        :m_Pos(pos),m_Stride(stride),m_BufferView(bufferView),
        m_Offset(offset),m_Model(model)
    {
    }
    AccessorIterator::AccessorIterator(const AccessorIterator& other)
        :m_BufferView(other.m_BufferView)
    {
        m_Pos=other.m_Pos;
        m_Stride=other.m_Stride;
        m_BufferView=other.m_BufferView;
        m_Offset=other.m_Offset;
        m_Model=other.m_Model;
    }
    Element& AccessorIterator::operator*()
    {
        auto& bufferView=m_Model->bufferViews[m_BufferView];
        auto& buffer=m_Model->buffers[bufferView.GetBuffer()];
        return *((Element*)(&buffer[m_Offset+m_Pos]));
    }
    bool AccessorIterator::operator==(const AccessorIterator& other)
    {
        return m_Pos==other.m_Pos&&
        m_Stride==other.m_Stride&&
        m_BufferView==other.m_BufferView&&
        m_Model==other.m_Model&&
        m_Offset==other.m_Offset;
    }
    AccessorIterator AccessorIterator::operator=(const AccessorIterator& other)
    {
        m_BufferView=other.m_BufferView;
        m_Pos=other.m_Pos;
        m_Stride=other.m_Stride;
        m_Model=other.m_Model;
        m_Offset=other.m_Offset;
        return *this;
    }
    AccessorIterator AccessorIterator::operator++(int i)//iter++
    {
        AccessorIterator old=*this;
        m_Pos+=m_Stride;
        return old;
    }
    AccessorIterator AccessorIterator::operator++()//++iter
    {
        m_Pos+=m_Stride;
        return *this;
    }
    AccessorIterator AccessorIterator::operator--()//--iter
    {
        m_Pos-=m_Stride;
        return *this;
    }
    AccessorIterator AccessorIterator::operator--(int i)//iter++
    {
        AccessorIterator old=*this;
        m_Pos-=m_Stride;
        return old;
    }
    bool AccessorIterator::operator!=(const AccessorIterator& other)
    {
        return !(operator==(other));
    }


   
    AccessorIterator Accessor::Begin()
    {
        return AccessorIterator(0,m_Stride,
        m_BufferView,m_Offset,m_Model);
    }
    AccessorIterator Accessor::End()
    {
        return AccessorIterator(m_Cnt*GetElementSize(m_ElementType),m_Stride,
        m_BufferView,m_Offset,m_Model);
    }
    AccessorIterator Accessor::begin()
    {
        return Begin();
    }
    AccessorIterator Accessor::end()
    {
        return End();
    }
    constexpr size_t Accessor::GetElementSize(ElementType type)
    {
        switch (type) 
        {
            case ElementType::REAL:
                return sizeof(Real);
                break;
            case ElementType::VEC2:
                return 2*sizeof(Real);
                break;
            case ElementType::VEC3:
                return 3*sizeof(Real);
                break;
            case ElementType::UNSIGNED_INT32:
                return 4;
                break;
            default:
                AETHER_ASSERT(false&&"unknown type");
        }
    }
    constexpr size_t Accessor::GetScalarSize(ElementType type)
    {
        switch (type) 
        {
            case ElementType::REAL:
                return sizeof(Real);
                break;
            case ElementType::VEC2:
                return sizeof(Real);
                break;
            case ElementType::VEC3:
                return sizeof(Real);
                break;
            case ElementType::UNSIGNED_INT32:
                return 4;
                break;
            default:
                AETHER_ASSERT(false&&"unknown type");
        }
    }
    Accessor::Accessor(size_t bufferViewIndex,size_t stride,ElementType elementType,
    size_t offset,Model* model)
        :m_ElementType(elementType),
        m_BufferView(bufferViewIndex),
        m_Stride(stride),
        m_Model(model),
        m_Offset(offset)

    {
        BufferView& bufferView=m_Model->bufferViews[m_BufferView];
        m_Cnt=(bufferView.GetEnd()-bufferView.GetBegin())/GetElementSize(elementType);
        
    }
}