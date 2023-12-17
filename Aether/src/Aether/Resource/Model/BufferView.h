#pragma once
#include <vector>
#include "../../Core/Core.h"
#include <optional>
#include "Aether/Render/VertexBuffer.h"
namespace Aether
{
    class Model;
    class BufferView//Buffer[begin,end) 
    {
        friend class Model;
    public:
        BufferView(size_t _buffer,size_t _begin,size_t _end)
            :m_Buffer(_buffer),m_Begin(_begin),m_End(_end){}
        BufferView(size_t _buffer,size_t _begin,size_t _end,Model* model)
            :m_Buffer(_buffer),m_Begin(_begin),m_End(_end),m_Model(model){}
        BufferView(const BufferView&)=default;
        BufferView(BufferView&&)=default;
        bool operator==(const BufferView& other)
        {
            return m_Buffer==other.m_Buffer&&m_Begin==other.m_Begin&&m_End==other.m_End;
        }
        BufferView operator=(const BufferView& other)
        {
            m_Buffer=other.m_Buffer;
            m_Begin=other.m_Begin;
            m_End=other.m_End;
            return *this;
        }
        inline size_t GetBuffer(){return m_Buffer;}
        inline size_t GetBegin(){return m_Begin;}
        inline size_t GetEnd(){return m_End;}
        
        void SetModel(Model* model)
        {
            m_Model=model;
        }
    private:
        size_t m_Buffer;
        size_t m_Begin;
        size_t m_End;
        Ref<VertexBuffer> m_VBO;
        Model* m_Model;
        void Bind();
        void Unbind();
    };
}