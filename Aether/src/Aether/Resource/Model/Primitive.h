#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include "Aether/Render/VertexArray.h"
#include "Aether/Render/IndexBuffer.h"
#include "Aether/Render/VertexBuffer.h"

namespace Aether
{
    class Model;
    class Primitive
    {
    public:
        Primitive()
        {

        }
        Primitive(Model* model)
            :m_Model(model)
        {}
        void Bind();
        void Unbind();
        inline std::unordered_map<std::string, size_t>& 
        GetAttributes()
        {
            return m_Attributes;
        }
        inline const std::unordered_map<std::string, size_t>& 
        GetAttributes()const
        {
            return m_Attributes;
        }
        inline std::optional<size_t> GetIndices()const{return m_Indices;}
        inline void SetIndices(size_t accessor){m_Indices=accessor;}
        inline void SetModel(Model* model){m_Model=model;}
        Ref<IndexBuffer> GetIndexBufferRef(){return m_IBO;}
        Ref<VertexArray> GetVertexArrayRef(){return m_VAO;}
        IndexBuffer& GetIndexBuffer(){return *m_IBO;}
        VertexArray& GetVertexArray(){return *m_VAO;}
    private:
        std::unordered_map<std::string, size_t> m_Attributes;
        std::optional<size_t> m_Indices;//accessor ref
        Model* m_Model;
        Ref<IndexBuffer> m_IBO;
        Ref<VertexArray> m_VAO;
        
    };
}