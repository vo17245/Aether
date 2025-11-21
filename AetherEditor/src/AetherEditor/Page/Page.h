#pragma once
#include <Core/Core.h>
#include <ImGui/ImGui.h>
#include <Debug/Log.h>
using namespace Aether;
namespace AetherEditor::UI
{

class PageRouter;
class Page
{
public:
    Page(const std::string& tag = "")
        :m_Tag(tag)
    {
    }
    virtual ~Page() = default;
    virtual void OnImGuiUpdate() {}
    void SetTag(const std::string& tag)
    {
        m_Tag = tag;
    }
    const std::string& GetTag() const
    {
        return m_Tag;
    }
    void SetPageRouter(PageRouter* router)
    {
        m_PageRouter = router;
    }
    PageRouter* GetPageRouter() const
    {
        return m_PageRouter;
    }
private:
    std::string m_Tag;
    PageRouter* m_PageRouter=nullptr;;

};
class PageRouter
{
public:
    template<typename T>
    requires std::is_base_of_v<Page, T>
    T* PushPage(Scope<T>&& page)
    {
        auto* ptr=page.get();
        m_Pages[page->GetTag()] =  std::move(page);
        return ptr;
    }
    void PopPage(const std::string& tag)
    {
        m_Pages.erase(tag);
    }
    Page* GetPage(const std::string& tag)
    {
        auto it = m_Pages.find(tag);
        if (it != m_Pages.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
    void NavigateTo(const std::string& tag)
    {
        auto page = GetPage(tag);
        if (page)
        {
            m_CurrentPage = page;
        }
        else
        {
            LogE("PageRouter: Page with tag '{}' not found!", tag);
        }
    }
    Page* GetCurrentPage() const
    {
        return m_CurrentPage;
    }
private:
    std::unordered_map<std::string, Scope<Page>> m_Pages;
    Page* m_CurrentPage = nullptr;
};
}