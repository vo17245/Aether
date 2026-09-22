#pragma once

#include <Render/Frame/RenderFrameContext.h>
#include <Render/Resource/RenderResourceHandle.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace Aether::Render
{
// Render-thread-owned storage. Release invalidates the logical handle
// immediately but defers physical destruction through a submission serial.
template <typename Resource, typename Tag = Resource>
class RenderResourceRegistry
{
public:
    using Handle = RenderResourceHandle<Tag>;

    void BindToCurrentThread()
    {
        if (m_Owner != std::thread::id{} && m_Owner != std::this_thread::get_id())
            throw std::logic_error("render resource registry already belongs to another thread");
        m_Owner = std::this_thread::get_id();
    }

    template <typename... Args>
    Handle Create(Args&&... args)
    {
        CheckOwner();
        std::uint32_t index;
        if (m_Free.empty())
        {
            index = static_cast<std::uint32_t>(m_Slots.size());
            m_Slots.emplace_back();
        }
        else
        {
            index = m_Free.back();
            m_Free.pop_back();
        }
        Slot& slot = m_Slots[index];
        slot.value.emplace(std::forward<Args>(args)...);
        slot.active = true;
        slot.lastUse = 0;
        ++m_Active;
        return {index, slot.generation};
    }

    Resource* TryGet(Handle handle)
    {
        CheckOwner();
        Slot* slot = Find(handle);
        return slot ? &*slot->value : nullptr;
    }

    const Resource* TryGet(Handle handle) const
    {
        CheckOwner();
        const Slot* slot = Find(handle);
        return slot ? &*slot->value : nullptr;
    }

    bool MarkUsed(Handle handle, SubmissionSerial serial)
    {
        CheckOwner();
        Slot* slot = Find(handle);
        if (!slot) return false;
        slot->lastUse = std::max(slot->lastUse, serial);
        return true;
    }

    bool Release(Handle handle, SubmissionSerial retireAfter = 0)
    {
        CheckOwner();
        Slot* slot = Find(handle);
        if (!slot) return false;
        slot->active = false;
        --m_Active;
        m_Retired.push_back({handle.index, handle.generation, std::max(slot->lastUse, retireAfter)});
        return true;
    }

    std::size_t Collect(SubmissionSerial completedSerial)
    {
        CheckOwner();
        std::size_t collected = 0;
        auto iterator = m_Retired.begin();
        while (iterator != m_Retired.end())
        {
            if (iterator->retireAfter > completedSerial)
            {
                ++iterator;
                continue;
            }
            Slot& slot = m_Slots[iterator->index];
            if (slot.generation == iterator->generation && !slot.active)
            {
                slot.value.reset();
                slot.lastUse = 0;
                ++slot.generation;
                if (slot.generation == 0) ++slot.generation;
                m_Free.push_back(iterator->index);
                ++collected;
            }
            iterator = m_Retired.erase(iterator);
        }
        return collected;
    }

    std::size_t ActiveCount() const noexcept { return m_Active; }
    std::size_t PendingRetirementCount() const noexcept { return m_Retired.size(); }

private:
    struct Slot
    {
        std::optional<Resource> value;
        std::uint32_t generation = 1;
        SubmissionSerial lastUse = 0;
        bool active = false;
    };
    struct Retired
    {
        std::uint32_t index;
        std::uint32_t generation;
        SubmissionSerial retireAfter;
    };

    void CheckOwner() const
    {
        if (m_Owner == std::thread::id{})
            const_cast<RenderResourceRegistry*>(this)->m_Owner = std::this_thread::get_id();
        else if (m_Owner != std::this_thread::get_id())
            throw std::logic_error("render resource registry accessed from a non-owner thread");
    }

    Slot* Find(Handle handle)
    {
        if (!handle || handle.index >= m_Slots.size()) return nullptr;
        Slot& slot = m_Slots[handle.index];
        return slot.active && slot.generation == handle.generation && slot.value ? &slot : nullptr;
    }

    const Slot* Find(Handle handle) const
    {
        if (!handle || handle.index >= m_Slots.size()) return nullptr;
        const Slot& slot = m_Slots[handle.index];
        return slot.active && slot.generation == handle.generation && slot.value ? &slot : nullptr;
    }

    mutable std::thread::id m_Owner{};
    std::vector<Slot> m_Slots;
    std::vector<std::uint32_t> m_Free;
    std::vector<Retired> m_Retired;
    std::size_t m_Active = 0;
};
} // namespace Aether::Render
