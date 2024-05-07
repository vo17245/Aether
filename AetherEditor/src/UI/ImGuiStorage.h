#pragma once

#include "Aether/ImGui.h"
#include <unordered_map>
#include <vector>
namespace Aether {
namespace Editor {
class ImGuiStorage
{
public:
    static constexpr const inline size_t MAX_STORAGE_SIZE = 1024;
    static char* Get(const ImGuiID id)
    {
        return GetInstance().GetImpl(id);
    }
    static void Clear()
    {
        GetInstance().ClearImpl();
    }

private:
    void ClearImpl()
    {
        std::unordered_map<ImGuiID, std::vector<char>>().swap(m_Data);
    }
    char* GetImpl(const ImGuiID id)
    {
        if (m_Data.find(id) == m_Data.end())
        {
            m_Data[id].resize(MAX_STORAGE_SIZE, 0);
        }
        return m_Data[id].data();
    }
    static ImGuiStorage& GetInstance()
    {
        static ImGuiStorage instance;
        return instance;
    }
    ImGuiStorage() = default;
    std::unordered_map<ImGuiID, std::vector<char>> m_Data;
};
}
} // namespace Aether::Editor