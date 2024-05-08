#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "Aether/Core.h"
namespace Aether {
namespace Editor {

class UICommandHandler
{
public:
    ~UICommandHandler() = default;
    UICommandHandler() = default;
    void RegisterCommand(const std::string& command, std::function<void()> handler)
    {
        m_Handlers[command] = handler;
    }
    bool ExecuteCommand(const std::string& command)
    {
        if (m_Handlers.find(command) != m_Handlers.end())
        {
            m_Handlers[command]();
            return true;
        }
        return false;
    }
    void PushCommand(const std::string& command)
    {
        m_Commands.push_back(command);
    }
    bool DoAllCommand()
    {
        bool any_failed = false;
        for (auto& command : m_Commands)
        {
            if (!ExecuteCommand(command))
            {
                AETHER_DEBUG_LOG_ERROR("Command not found: {0}", command);
                any_failed = true;
            }
        }
        m_Commands.clear();
        return !any_failed;
    }
    std::vector<std::string> m_Commands;
    std::unordered_map<std::string, std::function<void()>> m_Handlers;
};
}
} // namespace Aether::Editor
