#pragma once

#include <Render/Threads/RenderCommand.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Aether::Render
{
namespace Detail
{
struct CommandTicketState;
}

enum class CommandCompletionStatus
{
    Pending,
    Succeeded,
    Skipped,
    Cancelled,
    Failed,
};

struct CommandResult
{
    CommandCompletionStatus status = CommandCompletionStatus::Pending;
    std::string message;
};

enum class TicketWaitStatus
{
    Completed,
    Timeout,
    WouldDeadlock,
    Invalid,
};

class CommandTicket
{
public:
    CommandTicket() = default;

    bool IsValid() const noexcept;
    bool IsReady() const;
    std::uint64_t Id() const noexcept;
    TicketWaitStatus Wait() const;
    TicketWaitStatus WaitFor(std::chrono::milliseconds timeout) const;
    std::optional<CommandResult> TryGetResult() const;

private:
    explicit CommandTicket(std::shared_ptr<Detail::CommandTicketState> state);
    std::shared_ptr<Detail::CommandTicketState> m_State;
    friend class RenderThread;
};

struct RenderEnvelope
{
    CpuFrameId cpuFrameId = 0;
    std::vector<std::unique_ptr<IRenderCommand>> reliableCommands;
    std::unique_ptr<IRenderCommand> drawCommand;
    // Owned bytes not represented by a command's PayloadBytes(), for example
    // immutable snapshot arrays stored directly by an envelope implementation.
    std::size_t additionalPayloadBytes = 0;

    std::size_t CommandCount() const noexcept;
    std::size_t PayloadBytes() const noexcept;
    bool HasDraw() const noexcept { return static_cast<bool>(drawCommand); }
};

struct RenderThreadLimits
{
    std::size_t maxDrawOutstanding = 2;
    std::size_t maxCommandsOutstanding = 256;
    std::size_t maxPayloadBytes = 64u * 1024u * 1024u;
};

enum class RenderSubmitStatus
{
    Accepted,
    NotReady,
    Full,
    TooLarge,
    Invalid,
    Stopped,
    Failed,
};

struct RenderSubmitReceipt
{
    CommandTicket envelope;
    std::vector<CommandTicket> reliableCommands;
    std::optional<CommandTicket> draw;
};

struct RenderSubmitResult
{
    RenderSubmitStatus status = RenderSubmitStatus::NotReady;
    RenderSubmitReceipt receipt;
    std::string message;

    explicit operator bool() const noexcept { return status == RenderSubmitStatus::Accepted; }
};

enum class RenderThreadState
{
    Starting,
    Running,
    Stopping,
    Stopped,
    Failed,
};

struct RenderThreadStats
{
    std::size_t drawOutstanding = 0;
    std::size_t commandsOutstanding = 0;
    std::size_t payloadBytesOutstanding = 0;
    std::size_t queuedEnvelopes = 0;
    bool processingEnvelope = false;
};

class RenderThread
{
public:
    using LifecycleCallback = std::function<void()>;

    explicit RenderThread(RenderThreadLimits limits = {}, LifecycleCallback onStart = {},
                          LifecycleCallback onStop = {});
    ~RenderThread();
    RenderThread(const RenderThread&) = delete;
    RenderThread& operator=(const RenderThread&) = delete;

    bool Start();
    CommandTicket ReadyTicket() const;
    RenderSubmitResult TrySubmit(RenderEnvelope& envelope);
    bool WaitForCapacity(std::size_t commandCount, std::size_t payloadBytes, bool hasDraw,
                         std::chrono::milliseconds timeout);
    TicketWaitStatus FlushCpuThrough(const CommandTicket& ticket) const;
    void RequestStop();
    void Join();

    RenderThreadState State() const noexcept;
    RenderThreadStats Stats() const;
    bool IsRenderThread() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};
} // namespace Aether::Render
