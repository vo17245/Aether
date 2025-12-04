#pragma once

#include "Core/Core.h"
#include "Buffer.h"
#include "GraphicsCommandPool.h"
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <vector>
namespace Aether {
namespace vk {

template <typename T, size_t N>
/**
 * @brief 适用于频繁更新ssbo，但是每次更新几个元素而不是整个buffer的场景
 */
class SSBOArray
{
public:
    /**
     *   @brief 插入一个元素到数组，返回插入的索引
     */
    bool Push(const T& t, uint32_t& index)
    {
        // 如果数组未满
        if (m_Size < N)
        {
            m_HostBuffer[m_Size] = t;
            m_IsDeleted[m_Size] = false;
            m_Size++;
            index = m_Size - 1;
            m_ChangedIndices.emplace_back(index);
            return true;
        }
        // 如果数组已满

        // 找到第一个被删除的元素
        for (uint32_t i = 0; i < N; i++)
        {
            if (m_IsDeleted[i])
            {
                m_HostBuffer[i] = t;
                m_IsDeleted[i] = false;
                index = i;
                m_ChangedIndices.emplace_back(index);
                return true;
            }
        }
        return false;
    }
    bool Pop(uint32_t index)
    {
        if (index >= N)
        {
            return false;
        }
        m_IsDeleted[index] = true;
        return true;
    }
    bool Set(uint32_t index, const T& t)
    {
        if (index >= N)
        {
            return false;
        }
        m_ChangedIndices.emplace_back(index);
        m_HostBuffer[index] = t;
        return true;
    }

    constexpr size_t GetCapacity() const
    {
        return N;
    }
    bool SyncUpdate(GraphicsCommandPool& commandPool)
    {
        bool res = true;
        std::sort(m_ChangedIndices.begin(), m_ChangedIndices.end());
        m_ChangedIndices.erase(std::unique(m_ChangedIndices.begin(), m_ChangedIndices.end()), m_ChangedIndices.end());
        size_t begin = 0;
        size_t end = 1;
        // 全部拷贝到staging上
        m_StagingBuffer.SetData(reinterpret_cast<const uint8_t*>(m_HostBuffer), sizeof(T) * N);
        // 更新过的元素拷贝到device上
        while (end <= m_ChangedIndices.size())
        {
            if (end < m_ChangedIndices.size() && (m_ChangedIndices[end] == m_ChangedIndices[end - 1] + 1))
            {
                end++;
                continue;
            }
            res &= vk::Buffer::SyncCopy(commandPool, m_StagingBuffer,
                                        m_DeviceBuffer,
                                        sizeof(T) * (end - begin),
                                        sizeof(T) * m_ChangedIndices[begin],
                                        sizeof(T) * m_ChangedIndices[begin]);
            begin = end;
            end = begin + 1;
        }
        m_ChangedIndices.clear();
        return res;
    }
    static Ref<SSBOArray<T, N>> CreateRef()
    {
        auto stagingBuffer = vk::Buffer::CreateForStaging(sizeof(T) * N);
        if (!stagingBuffer.has_value())
        {
            return nullptr;
        }
        auto deviceBuffer = vk::Buffer::CreateForSSBO(sizeof(T) * N);
        if (!deviceBuffer.has_value())
        {
            return nullptr;
        }
        auto* ptr = new SSBOArray<T, N>(std::move(stagingBuffer.value()), std::move(deviceBuffer.value()));
        return Ref<SSBOArray<T, N>>(ptr);
    }
    static Scope<SSBOArray<T, N>> CreateScope()
    {
        auto stagingBuffer = vk::Buffer::CreateForStaging(sizeof(T) * N);
        if (!stagingBuffer.has_value())
        {
            return nullptr;
        }
        auto deviceBuffer = vk::Buffer::CreateForSSBO(sizeof(T) * N);
        if (!deviceBuffer.has_value())
        {
            return nullptr;
        }
        auto* ptr = new SSBOArray<T, N>(std::move(stagingBuffer.value()), std::move(deviceBuffer.value()));
        return Scope<SSBOArray<T, N>>(ptr);
    }
    vk::Buffer& GetDeviceBuffer()
    {
        return m_DeviceBuffer;
    }
    size_t GetSize() const
    {
        return m_Size;
    }
    void Resize(size_t size)
    {
        m_Size = size;
    }
    bool ShouldUpdate() const
    {
        return !m_ChangedIndices.empty();
    }
    SSBOArray(const SSBOArray&) = delete;
    SSBOArray& operator=(const SSBOArray&) = delete;
    SSBOArray(SSBOArray&& other) = delete;
    SSBOArray& operator=(SSBOArray&& other) = delete;

private:
    SSBOArray(vk::Buffer&& stagingBuffer, vk::Buffer&& deviceBuffer) :
        m_StagingBuffer(std::move(stagingBuffer)), m_DeviceBuffer(std::move(deviceBuffer))
    {
    }
    vk::Buffer m_StagingBuffer;
    vk::Buffer m_DeviceBuffer;
    size_t m_Size = 0;
    std::vector<size_t> m_ChangedIndices;

private: // 让m_HostBuffer定义在最后，提高cache命中率
    bool m_IsDeleted[N];
    T m_HostBuffer[N];
};
}
} // namespace Aether::vk