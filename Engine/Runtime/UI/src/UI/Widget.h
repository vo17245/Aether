#pragma once
#include <Core/Core.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <Window/Event.h>
namespace Aether
{
class Widget;
class WidgetFocusManager
{
public:
    static bool IsWidgetFocused(Widget* widget)
    {
        return Get().m_FocusedWidget == widget;
    }
    static void SetWidgetFocused(Widget* widget)
    {
        Get().m_FocusedWidget = widget;
    }

private:
    static WidgetFocusManager& Get()
    {
        static WidgetFocusManager instance;
        return instance;
    }
    Widget* m_FocusedWidget = nullptr;
};
class WidgetMouseCaptureManager
{
public:
    static Widget* GetCapturedWidget()
    {
        return Get().m_CapturedWidget;
    }
    static void SetCapturedWidget(Widget* widget)
    {
        Get().m_CapturedWidget = widget;
    }

private:
    static WidgetMouseCaptureManager& Get()
    {
        static WidgetMouseCaptureManager instance;
        return instance;
    }
    Widget* m_CapturedWidget = nullptr;
};
class Widget
{
public:
    virtual ~Widget() = default;
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(Widget&&) = delete;

public:
    // 位置和尺寸
    void SetPosition(const Vec2f& pos)
    {
        m_Position = pos;
        OnLayoutRecursive();
    }
    Vec2f GetPosition() const
    {
        return m_Position;
    }
    Vec2f GetAbsolutePosition() const
    {
        return m_AbsolutePosition;
    }

    virtual void SetSize(const Vec2f& size)
    {
        m_Size = size;
        OnLayoutRecursive();
    }
    virtual Vec2f GetSize() const
    {
        return m_Size;
    }

    // 可见性
    void SetVisible(bool visible)
    {
        m_Visible = visible;
    }
    virtual bool IsVisible() const
    {
        return m_Visible;
    }

    // 启用/禁用状态(禁用时不处理事件)
    void SetEnabled(bool enabled)
    {
        m_Enabled = enabled;
    }
    bool IsEnabled() const
    {
        return m_Enabled;
    }
    // 渲染相关
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph)
    {
    }
    virtual bool NeedRebuildRenderGraph() const
    {
        return false;
    }
    // 事件
    virtual bool HitTest(const Vec2f& point) const
    {
        const Vec2f& abs = m_AbsolutePosition;
        return point.x() >= abs.x() && point.y() >= abs.y() && point.x() <= abs.x() + m_Size.x()
               && point.y() <= abs.y() + m_Size.y();
    }
    virtual bool OnKeyboardPress(KeyboardPressEvent& event)
    {
        return false;
    }
    virtual bool OnKeyboardRelease(KeyboardReleaseEvent& event)
    {
        return false;
    }
    virtual bool OnMouseButtonPressed(MouseButtonPressedEvent& event)
    {
        return false;
    }
    virtual bool OnMouseButtonReleased(MouseButtonReleasedEvent& event)
    {
        return false;
    }
    bool OnEventRecursive(Event& event)
    {
        if (!m_Visible || !m_Enabled)
        {
            return false;
        }
        for (auto iter = m_Children.rbegin(); iter != m_Children.rend(); ++iter)
        {
            auto& child = *iter;
            if (child->OnEventRecursive(event))
            {
                return true;
            }
        }
        return std::visit(
            [this](auto& e) {
                using T = std::decay_t<decltype(e)>;
                if constexpr (std::is_same_v<T, KeyboardPressEvent>)
                {
                    if (!CanReceiveFocus() || !HasFocus())
                    {
                        return false;
                    }
                    return this->OnKeyboardPress(e);
                }
                else if constexpr (std::is_same_v<T, KeyboardReleaseEvent>)
                {
                    if (!CanReceiveFocus() || !HasFocus())
                    {
                        return false;
                    }
                    WidgetMouseCaptureManager::SetCapturedWidget(nullptr);
                    return this->OnKeyboardRelease(e);
                }
                else if constexpr (std::is_same_v<T, MouseButtonPressedEvent>)
                {
                    if (!HitTest(e.GetPosition()))
                    {
                        return false;
                    }
                    WidgetFocusManager::SetWidgetFocused(this);
                    WidgetMouseCaptureManager::SetCapturedWidget(this);

                    return this->OnMouseButtonPressed(e);
                }
                else if constexpr (std::is_same_v<T, MouseButtonReleasedEvent>)
                {
                    if (!HitTest(e.GetPosition()))
                    {
                        return false;
                    }
                    return this->OnMouseButtonReleased(e);
                }
                else
                {
                    return false;
                }
            },
            event);
    }
    virtual void OnUpdate(float deltaSec)
    {
    }
    void OnUpdateRecursive(float deltaSec)
    {
        OnUpdate(deltaSec);
        for (auto& child : m_Children)
        {
            child->OnUpdateRecursive(deltaSec);
        }
    }
    // 布局相关
    void OnLayout()
    {
        m_AbsolutePosition = m_Parent ? m_Parent->m_AbsolutePosition + m_Position : m_Position;
    }
    void OnLayoutRecursive()
    {
        OnLayout();
        for (auto& child : m_Children)
        {
            child->OnLayoutRecursive();
        }
    }
    // 层级关系
    Widget* GetParent() const
    {
        return m_Parent;
    }
    void SetParent(Widget* parent)
    {
        m_Parent = parent;
        OnLayoutRecursive();
    }
    void AddChild(Scope<Widget>&& child)
    {
        child->SetParent(this);
        m_Children.push_back(std::move(child));
    }

    virtual bool CanReceiveFocus() const
    {
        return false;
    }
    virtual bool HasFocus() const
    {
        return WidgetFocusManager::IsWidgetFocused(const_cast<Widget*>(this));
    }

private:
    Widget* m_Parent = nullptr;
    std::vector<Scope<Widget>> m_Children;
    bool m_Visible = true;
    bool m_Enabled = true;
    Vec2f m_Position = Vec2f(0.0f, 0.0f); // relative position to parent
    Vec2f m_AbsolutePosition = Vec2f(0.0f, 0.0f);
    Vec2f m_Size = Vec2f(0.0f, 0.0f);
};
} // namespace Aether