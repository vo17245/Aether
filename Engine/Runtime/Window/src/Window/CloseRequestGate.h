#pragma once

#include <functional>
#include <utility>

namespace Aether
{
// SDL/Vulkan-independent close policy, testable without a GPU window.
class CloseRequestGate final
{
public:
    void SetHandler(std::function<void()> handler) { m_Handler = std::move(handler); }
    void Request()
    {
        if (!m_Handler) { m_ShouldClose = true; return; }
        if (m_RequestPending) return;
        m_RequestPending = true;
        m_Handler();
    }
    void Confirm() noexcept { m_RequestPending = false; m_ShouldClose = true; }
    void Cancel() noexcept { m_RequestPending = false; }
    bool ShouldClose() const noexcept { return m_ShouldClose; }
    bool RequestPending() const noexcept { return m_RequestPending; }
private:
    std::function<void()> m_Handler;
    bool m_RequestPending = false;
    bool m_ShouldClose = false;
};
}
