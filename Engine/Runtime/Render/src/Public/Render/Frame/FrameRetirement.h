#pragma once

#include <Render/Frame/RenderFrameContext.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

namespace Aether::Render
{
class FrameRetirement
{
public:
    template <typename T>
    void Retain(SubmissionSerial serial, std::shared_ptr<T> object)
    {
        if (!object) return;
        auto iterator = std::find_if(m_Batches.begin(), m_Batches.end(),
                                     [serial](const Batch& batch) { return batch.serial == serial; });
        if (iterator == m_Batches.end())
        {
            m_Batches.push_back({serial, {}});
            iterator = std::prev(m_Batches.end());
        }
        iterator->objects.emplace_back(std::move(object));
    }

    std::size_t Collect(SubmissionSerial completedSerial)
    {
        std::size_t count = 0;
        auto iterator = m_Batches.begin();
        while (iterator != m_Batches.end())
        {
            if (iterator->serial <= completedSerial)
            {
                count += iterator->objects.size();
                iterator = m_Batches.erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }
        return count;
    }

    std::size_t RetainedCount() const noexcept
    {
        std::size_t count = 0;
        for (const auto& batch : m_Batches) count += batch.objects.size();
        return count;
    }

private:
    struct Batch
    {
        SubmissionSerial serial;
        std::vector<std::shared_ptr<void>> objects;
    };
    std::vector<Batch> m_Batches;
};
} // namespace Aether::Render
