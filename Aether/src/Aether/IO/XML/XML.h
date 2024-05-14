#pragma once
#include "tinyxml2.h"
#include <optional>
#include <memory>
#include <string>
#include "Aether/Core/Log.h"
namespace Aether {
class XML
{
public:
    //========================= element ==================
    class Element
    {
        //========================= attribute ==================
        friend class XML;

    public:
        std::string GetText()
        {
            return m_Ptr->GetText();
        }
        std::optional<std::string> GetAttribute(const std::string& name)
        {
            const char* value = nullptr;
            m_Ptr->QueryStringAttribute(name.c_str(), &value);
            if (value == nullptr)
            {
                return std::nullopt;
            }
            return value;
        }
        class Attribute
        {
        public:
            std::string GetName() const
            {
                return m_Ptr->Name();
            }
            std::string GetValue() const
            {
                return m_Ptr->Value();
            }
            Attribute(tinyxml2::XMLAttribute* ptr) :
                m_Ptr(ptr)
            {}

        private:
            tinyxml2::XMLAttribute* m_Ptr = nullptr;
        };
        class AttributeIterator
        {
        public:
            AttributeIterator(const AttributeIterator& other) :
                m_Ptr(other.m_Ptr)
            {}
            AttributeIterator& operator=(const AttributeIterator& other)
            {
                m_Ptr = other.m_Ptr;
                return *this;
            }

            AttributeIterator(tinyxml2::XMLAttribute* ptr) :
                m_Ptr(ptr)
            {}
            AttributeIterator& operator++()
            {
                m_Ptr = const_cast<tinyxml2::XMLAttribute*>(m_Ptr->Next());
                return *this;
            }
            bool operator!=(const AttributeIterator& other) const
            {
                return m_Ptr != other.m_Ptr;
            }
            Attribute operator*()
            {
                return Attribute(m_Ptr);
            }

        private:
            tinyxml2::XMLAttribute* m_Ptr = nullptr;
        };
        class AttributeIterable
        {
        public:
            AttributeIterable(tinyxml2::XMLAttribute* ptr) :
                m_Ptr(ptr)
            {}
            AttributeIterator begin()
            {
                return AttributeIterator(const_cast<tinyxml2::XMLAttribute*>(m_Ptr));
            }
            AttributeIterator end()
            {
                return AttributeIterator(nullptr);
            }

        private:
            tinyxml2::XMLAttribute* m_Ptr;
        };
        //========================= attribute end ==================
        AttributeIterable EachAttribute()
        {
            return AttributeIterable(const_cast<tinyxml2::XMLAttribute*>(m_Ptr->FirstAttribute()));
        }
        Element(tinyxml2::XMLElement* ptr) :
            m_Ptr(ptr)
        {}
        std::string GetName() const
        {
            return m_Ptr->Name();
        }

    private:
        tinyxml2::XMLElement* m_Ptr = nullptr;
    };
    //========================= element end ==================
    //========================= element iterator ==================
    class DepthFirstElementIterator
    {
    public:
        DepthFirstElementIterator(tinyxml2::XMLElement* ptr) :
            m_Ptr(ptr)
        {}
        DepthFirstElementIterator& operator++()
        {
            // 如果有子节点，返回第一个子节点
            if (m_Ptr->FirstChildElement() != nullptr)
            {
                m_Ptr = m_Ptr->FirstChildElement();
                return *this;
            }
            // 如果有兄弟节点，返回下一个兄弟节点
            if (m_Ptr->NextSiblingElement() != nullptr)
            {
                m_Ptr = m_Ptr->NextSiblingElement();
                return *this;
            }
            // 如果有父节点，返回父节点的下一个兄弟节点
            while (m_Ptr->Parent()->ToElement() != nullptr)
            {
                m_Ptr = m_Ptr->Parent()->ToElement();

                if (m_Ptr->NextSiblingElement() != nullptr)
                {
                    m_Ptr = m_Ptr->NextSiblingElement();
                    return *this;
                }
            }
            m_Ptr = nullptr;

            return *this;
        }
        bool operator!=(const DepthFirstElementIterator& other) const
        {
            return m_Ptr != other.m_Ptr;
        }
        Element operator*()
        {
            return Element(m_Ptr);
        }

    private:
        tinyxml2::XMLElement* m_Ptr = nullptr;
    };
    class DepthFirstElementIterable
    {
    public:
        DepthFirstElementIterable(tinyxml2::XMLElement* ptr) :
            m_Ptr(ptr)
        {}
        DepthFirstElementIterator begin()
        {
            return DepthFirstElementIterator(m_Ptr);
        }
        DepthFirstElementIterator end()
        {
            return DepthFirstElementIterator(nullptr);
        }

    private:
        tinyxml2::XMLElement* m_Ptr = nullptr;
    };

    DepthFirstElementIterable EachElement()
    {
        return DepthFirstElementIterable(m_Root);
    }
    class ChildrenElementIterator
    {
    public:
        ChildrenElementIterator(tinyxml2::XMLElement* firstChild)
            : m_Ptr(firstChild)
        {
        }
        ChildrenElementIterator& operator++()
        {
            m_Ptr = m_Ptr->NextSiblingElement();
            return *this;
        }
        Element operator*()
        {
            return Element(m_Ptr);
        }
        bool operator!=(const ChildrenElementIterator& other) const
        {
            return m_Ptr != other.m_Ptr;
        }

    private:
        tinyxml2::XMLElement* m_Ptr = nullptr;
    };
    class ChildrenElementIterable
    {
    public:
        ChildrenElementIterable(tinyxml2::XMLElement* parent) :
            m_Ptr(parent)
        {}
        ChildrenElementIterator begin()
        {
            auto res = ChildrenElementIterator(m_Ptr->FirstChildElement());
            return res;
        }
        ChildrenElementIterator end()
        {
            return ChildrenElementIterator(nullptr);
        }

    private:
        tinyxml2::XMLElement* m_Ptr = nullptr;
    };
    static ChildrenElementIterable EachChildren(Element element)
    {
        return ChildrenElementIterable(element.m_Ptr);
    }
    //========================= element iterator end==================
public:
    static std::optional<XML> LoadFile(const std::string& path)
    {
        XML xml;
        xml.m_Doc = std::make_unique<tinyxml2::XMLDocument>();
        auto res = xml.m_Doc->LoadFile(path.c_str());
        if (res != tinyxml2::XML_SUCCESS)
        {
            AETHER_DEBUG_LOG_ERROR("failed to load xml {},tinyxml2 error {}", path, int(res));
            return std::nullopt;
        }
        xml.m_Root = xml.m_Doc->RootElement();
        if (xml.m_Root == nullptr)
        {
            AETHER_DEBUG_LOG_ERROR("failed to load xml {},root is null", path);
            return std::nullopt;
        }
        xml.m_LoadFilePath = path;
        return xml;
    }
    XML(const XML&) = delete;
    XML& operator=(const XML&) = delete;
    XML(XML&& other)
    {
        m_Root = other.m_Root;
        m_LoadFilePath = std::move(other.m_LoadFilePath);
        m_Doc = std::move(other.m_Doc);
    }
    XML& operator=(XML&& other)
    {
        m_Root = other.m_Root;
        m_LoadFilePath = std::move(other.m_LoadFilePath);
        m_Doc = std::move(other.m_Doc);
        return *this;
    }
    ~XML() = default;
    Element GetRoot()
    {
        return Element(m_Root);
    }

private:
    XML() = default;

    tinyxml2::XMLElement* m_Root = nullptr;
    std::optional<std::string> m_LoadFilePath;
    std::unique_ptr<tinyxml2::XMLDocument> m_Doc;
};
} // namespace Aether
