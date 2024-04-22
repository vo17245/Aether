#pragma once
#include <vector>
#include "../../Core/Core.h"
#include <optional>
#include "Aether/Render/IndexBuffer.h"
#include "Aether/Render/VertexBuffer.h"
namespace Aether
{
    class Model;
    
    class BufferView//Buffer[begin,end) 
    {
        friend class Model;
    public:
        enum class Target
        {
            VERTEX_BUFFER,
            INDEX_BUFFER,
        };
        
        BufferView(size_t _buffer,size_t _begin,size_t _end,
        Target target,Model* model)
            :m_Buffer(_buffer),m_Begin(_begin),m_End(_end),m_Model(model),
            m_Target(target)
        {}
        BufferView(const BufferView&)=delete;
        BufferView(BufferView&&)=default;
        BufferView& operator=(const BufferView&)=delete;
        BufferView& operator=(BufferView&&)=default;
       
       
        inline size_t GetBuffer(){return m_Buffer;}
        inline size_t GetBegin(){return m_Begin;}
        inline size_t GetEnd(){return m_End;}
        inline Target GetTarget(){return m_Target;}
        inline Ref<VertexBuffer> GetVertexBuffer(){return m_VBO;}
        inline Ref<IndexBuffer> GetIndexBuffer(){return m_IBO;}
    private:
        size_t m_Buffer;
        size_t m_Begin;
        size_t m_End;
        Ref<VertexBuffer> m_VBO;
        Model* m_Model;
        void Bind();
        void Unbind();
        Target m_Target;
        Ref<IndexBuffer> m_IBO;
    };
}