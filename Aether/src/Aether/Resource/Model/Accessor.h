#pragma once
#include "Aether/Core/Math.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Buffer.h"
#include <stdint.h>
#include "Aether/Core/Core.h"
#include "Aether/Core/Assert.h"
namespace Aether
{
    enum class ElementType: uint32_t
    {   
        VEC3=Bit(0),
        VEC2=Bit(1),
        REAL=Bit(2),
        UNSIGNED_INT32=Bit(3),
    };
    struct Element
    {
        union
        {
            Real real;
            Vec3 vec3;
            Vec2 vec2;
            uint32_t ui32;
        };
    };
    class Model;
    class AccessorIterator
    {
    public:
        
        AccessorIterator(size_t pos,size_t stride,size_t bufferView,size_t offset,Model* model);
        AccessorIterator(const AccessorIterator& other);
        ~AccessorIterator(){}
        Element& operator*();
        bool operator==(const AccessorIterator& other);
        AccessorIterator operator=(const AccessorIterator& other);
        AccessorIterator operator++(int i);//iter++
        AccessorIterator operator++();//++iter
        AccessorIterator operator--();//--iter
        AccessorIterator operator--(int i);//iter--
        bool operator!=(const AccessorIterator& other);
    private:
        size_t m_Pos;
        size_t m_Stride;
        size_t m_BufferView;
        size_t m_Offset;
        Model* m_Model;
        
    };
    
    class Accessor
    {
    public:
        Accessor(size_t bufferViewIndex,size_t stride,ElementType elementType,size_t offset,
        Model* model);
        AccessorIterator Begin();
        AccessorIterator End();
        AccessorIterator begin();
        AccessorIterator end();
        constexpr size_t GetElementSize(ElementType type);
        constexpr size_t GetScalarSize(ElementType type);
        inline ElementType GetElementType()const{return m_ElementType;}
        inline size_t GetBufferView()const {return m_BufferView;}
        inline size_t GetStride()const{return m_Stride;}
        inline size_t GetOffset()const{return m_Offset;}
    private:
        ElementType m_ElementType;
        size_t m_BufferView;
        size_t m_Stride;
        size_t m_Cnt;
        Model* m_Model;
        size_t m_Offset;
    };
}