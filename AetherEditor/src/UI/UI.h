#pragma once
#include <variant>
#include "Aether/Event.h"
#include <functional>
namespace Aether {
namespace Editor {
namespace UI {
class UI
{
protected:
    virtual ~UI()
    {
        for (auto child : m_Children)
        {
            delete child;
        }
    }

public:
    void Destory()
    {
        delete this;
    }

public:
    UI() :
        m_Parent(nullptr){};
    UI(UI* parent) :
        m_Parent(parent){};

    void SetParent(UI* parent)
    {
        m_Parent = parent;
        parent->m_Children.push_back(this);
    }
    UI* GetParent()
    {
        return m_Parent;
    }
    void AddChild(UI* child)
    {
        child->SetParent(this);
    }
    void RemoveChild(UI* child)
    {
        for (auto iter = m_Children.begin(); iter != m_Children.end();)
        {
            if (*iter == child)
            {
                iter = m_Children.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }
    void RemoveFromParent()
    {
        if (m_Parent)
        {
            m_Parent->RemoveChild(this);
        }
    }

public:
    virtual void OnRender()
    {}
    virtual void OnEvent(Event& event)
    {}
    virtual void OnImGuiRender()
    {}
    virtual void OnUpdate(float sec)
    {}
    virtual void OnAttach()
    {}
    virtual void OnDetach()
    {}

private:
    UI* m_Parent;
    std::vector<UI*> m_Children;
};

}

}
} // namespace Aether::Editor::UI