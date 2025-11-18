#pragma once
#include <vector>
#include <string>
#include <cassert>
#include <Core/Core.h>
using namespace Aether;
namespace AetherEditor
{

enum class MessageType : uint8_t
{
    PageNavigation,
};
struct Message
{
    MessageType Type;
    Message(MessageType type) : Type(type)
    {
    }
};
struct PageNavigationMessage : public Message
{
    std::string TargetPage;
    PageNavigationMessage(const std::string& targetPage) : Message(MessageType::PageNavigation), TargetPage(targetPage)
    {
    }
};
class GlobalMessageBus
{
public:
    struct SubscriberSignature
    {
        MessageType Type;
        uint32_t Id;
    };
    static void Initialize()
    {
        assert(s_Instance == nullptr && "GlobalMessageBus is already initialized!");
        s_Instance = new GlobalMessageBus();
    }
    static void Shutdown()
    {
        assert(s_Instance != nullptr && "GlobalMessageBus is not initialized!");
        delete s_Instance;
        s_Instance = nullptr;
    }
    template <typename T, typename... Args>
        requires std::is_base_of_v<Message, T>
    static void Publish(Args&&... args)
    {
        GetSingleton().PublishImpl<T>(std::forward<Args>(args)...);
    }
    static SubscriberSignature Subscribe(MessageType type, const std::function<void(const Message&)>& handler)
    {
        return GetSingleton().SubscribeImpl(type, handler);
    }
    static void ProcessMessages()
    {
        GetSingleton().ProcessMessagesImpl();
    }
    static void Unsubscribe(const SubscriberSignature& signature)
    {
        GetSingleton().UnsubscribeImpl(signature);
    }

private:
    static GlobalMessageBus& GetSingleton()
    {
        assert(s_Instance != nullptr && "GlobalMessageBus is not initialized!");
        return *s_Instance;
    }
    template <typename T, typename... Args>
        requires std::is_base_of_v<Message, T>
    void PublishImpl(Args&&... args)
    {
        m_Messages.push_back(CreateScope<T>(std::forward<Args>(args)...));
    }
    SubscriberSignature SubscribeImpl(MessageType type, const std::function<void(const Message&)>& handler)
    {
        Entry entry{
            .Id = m_IdPool.Allocate(),
            .Handler = handler,
        };
        m_Subscribers[type].push_back(std::move(entry));
        return SubscriberSignature{type, entry.Id};
    }
    void ProcessMessagesImpl()
    {
        for (auto& message : m_Messages)
        {
            auto it = m_Subscribers.find(message->Type);
            if (it != m_Subscribers.end())
            {
                for (auto& entry : it->second)
                {
                    entry.Handler(*message);
                }
            }
        }
        m_Messages.clear();
    }
    void UnsubscribeImpl(const SubscriberSignature& signature)
    {
        auto it = m_Subscribers.find(signature.Type);
        if (it != m_Subscribers.end())
        {
            auto& entries = it->second;
            entries.erase(std::remove_if(entries.begin(), entries.end(),
                                         [&signature](const Entry& entry) { return entry.Id == signature.Id; }),
                          entries.end());
            m_IdPool.Deallocate(signature.Id);
        }
    }

private:
    struct IdPool
    {
        uint32_t NextId = 0;
        std::vector<uint32_t> FreeIds;
        uint32_t Allocate()
        {
            if (!FreeIds.empty())
            {
                uint32_t id = FreeIds.back();
                FreeIds.pop_back();
                return id;
            }
            return NextId++;
        }
        void Deallocate(uint32_t id)
        {
            FreeIds.push_back(id);
        }
    };

private:
    static GlobalMessageBus* s_Instance;
    std::vector<Scope<Message>> m_Messages;
    IdPool m_IdPool;
    struct Entry
    {
        uint32_t Id;
        std::function<void(const Message&)> Handler;
    };

    std::unordered_map<MessageType, std::vector<Entry>> m_Subscribers;
};

} // namespace AetherEditor
