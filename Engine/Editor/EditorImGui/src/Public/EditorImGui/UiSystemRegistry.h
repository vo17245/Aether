#pragma once

#include <World/World.h>

#include <algorithm>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Aether::EditorImGui
{
class UiIntentSink
{
public:
    virtual ~UiIntentSink() = default;
    virtual void Submit(std::string type, Json payload) = 0;
};

class UiSystemRegistry
{
public:
    using DrawCallback = std::function<void(const World&, UiIntentSink&)>;
    bool Register(std::string id, DrawCallback callback)
    {
        if (id.empty() || !callback || Contains(id)) return false;
        m_Systems.emplace_back(std::move(id), std::move(callback));
        return true;
    }
    bool Unregister(std::string_view id)
    {
        const auto oldSize = m_Systems.size();
        std::erase_if(m_Systems, [&](const auto& entry) { return entry.first == id; });
        return m_Systems.size() != oldSize;
    }
    bool Contains(std::string_view id) const
    {
        return std::any_of(m_Systems.begin(), m_Systems.end(),
            [&](const auto& entry) { return entry.first == id; });
    }
    std::size_t Size() const noexcept { return m_Systems.size(); }
    void Draw(const World& editorWorld, UiIntentSink& intents) const
    {
        for (const auto& [id, callback] : m_Systems) { (void)id; callback(editorWorld, intents); }
    }
private:
    std::vector<std::pair<std::string, DrawCallback>> m_Systems;
};
}
