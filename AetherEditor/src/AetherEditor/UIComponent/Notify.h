#pragma once
#include <Imgui/Compat/ImGuiApi.h>
#include <ImGui/Core/imgui.h>
#include <string>
#include <vector>
using namespace Aether;
enum class MessageType
{
    Info,
    Warning,
    Error,
    Debug
};
struct Message
{
    MessageType type;
    std::string content;
    float ttl; // time to live in seconds
};
class Notify
{
public:
    static Notify& GetSingleton()
    {
        static Notify instance;
        return instance;
    }
    static void Error(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().ErrorImpl(content, ttl);
    }
    static void Warning(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().WarningImpl(content, ttl);
    }
    static void Info(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().InfoImpl(content, ttl);
    }
    static void Draw()
    {
        GetSingleton().DrawImpl();
    }
    static void Debug(const std::string& content, float ttl = 5.0f)
    {
        GetSingleton().DebugImpl(content, ttl);
    }
    static void Update(float sec)
    {
        GetSingleton().UpdateImpl(sec);
    }
private:
    void UpdateImpl(float sec)
    {
        for (auto it = m_Messages.begin(); it != m_Messages.end();)
        {
            it->ttl -= sec;
            if (it->ttl <= 0.0f)
            {
                it = m_Messages.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    void DrawImpl()
    {
        if (m_Messages.empty())
        {
            return;
        }
        for (size_t i = 0; i < m_Messages.size(); ++i)
        {
            auto& msg = m_Messages[i];
            ImVec4 color;
            switch (msg.type)
            {
            case MessageType::Info:
                color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case MessageType::Warning:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case MessageType::Error:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            case MessageType::Debug:
                color = ImVec4(0.5f, 0.7f, 1.0f, 1.0f);
                break;
            default:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::GetForegroundDrawList()->AddText(ImVec2(10, 10 + 20 * i), ImColor(color), msg.content.c_str());
            ImGui::PopStyleColor();
        }
    }
    void ErrorImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Error, content, ttl});
    }
    void WarningImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Warning, content, ttl});
    }
    void InfoImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Info, content, ttl});
    }
    void DebugImpl(const std::string& content, float ttl = 5.0f)
    {
        m_Messages.push_back({MessageType::Debug, content, ttl});
    }
    std::vector<Message> m_Messages;
};