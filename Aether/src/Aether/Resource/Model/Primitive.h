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
        ~Primitive();
        //create vertex array and bind
        void Bind();
        //release vertex array
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
        Ref<VertexArray> GetVertexArrayRef(){return m_VAO;}
        VertexArray& GetVertexArray(){return *m_VAO;}
        bool HasNormal()
        {
            return HasAttribute("NORMAL");
        }
        bool HasPosition()
        {
            return HasAttribute("POSITION");
        }
        void AddAttribute(const std::string& name,size_t accessor_index)
        {
            m_Attributes[name]=accessor_index;
        }
    private:
        bool HasAttribute(const std::string& attribute)
        {
            auto iter = m_Attributes.find(attribute);
            return iter != m_Attributes.end();
        }
        std::unordered_map<std::string, size_t> m_Attributes;
        std::optional<size_t> m_Indices;//accessor ref
        Model* m_Model;
        Ref<VertexArray> m_VAO;
        
    };
}