#pragma once

#include <Render/Frame/RenderFrameContext.h>

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace Aether::Render
{
class IRenderCommand
{
public:
    virtual ~IRenderCommand() = default;
    virtual void Execute(RenderFrameContext& context) = 0;
    virtual std::size_t PayloadBytes() const noexcept { return 0; }
};

template <typename Fn>
class OwnedRenderCommand final : public IRenderCommand
{
public:
    OwnedRenderCommand(Fn fn, std::size_t payloadBytes)
        : m_Fn(std::move(fn)), m_PayloadBytes(payloadBytes)
    {
    }

    void Execute(RenderFrameContext& context) override
    {
        if constexpr (std::invocable<Fn&, RenderFrameContext&>)
            std::invoke(m_Fn, context);
        else
            std::invoke(m_Fn);
    }

    std::size_t PayloadBytes() const noexcept override { return m_PayloadBytes; }

private:
    Fn m_Fn;
    std::size_t m_PayloadBytes = 0;
};

template <typename Fn>
std::unique_ptr<IRenderCommand> MakeRenderCommand(Fn&& fn, std::size_t payloadBytes = 0)
{
    using Command = OwnedRenderCommand<std::decay_t<Fn>>;
    return std::make_unique<Command>(std::forward<Fn>(fn), payloadBytes);
}
} // namespace Aether::Render
