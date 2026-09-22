#include <Render/Threads/RenderThread.h>

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <exception>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace Aether::Render
{
namespace Detail
{
struct WorkerIdentity
{
    mutable std::mutex mutex;
    std::thread::id id{};

    bool IsCurrent() const
    {
        std::lock_guard lock(mutex);
        return id != std::thread::id{} && id == std::this_thread::get_id();
    }
};

struct CommandTicketState
{
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::uint64_t id = 0;
    CommandResult result;
    std::shared_ptr<WorkerIdentity> worker;

    bool Complete(CommandCompletionStatus status, std::string message = {})
    {
        {
            std::lock_guard lock(mutex);
            if (result.status != CommandCompletionStatus::Pending) return false;
            result.status = status;
            result.message = std::move(message);
        }
        condition.notify_all();
        return true;
    }
};
} // namespace Detail

namespace
{
std::string CurrentExceptionMessage()
{
    try
    {
        throw;
    }
    catch (const std::exception& exception)
    {
        return exception.what();
    }
    catch (...)
    {
        return "unknown render thread exception";
    }
}

std::size_t AddSaturated(std::size_t left, std::size_t right)
{
    if (right > std::numeric_limits<std::size_t>::max() - left)
        return std::numeric_limits<std::size_t>::max();
    return left + right;
}
} // namespace

CommandTicket::CommandTicket(std::shared_ptr<Detail::CommandTicketState> state) : m_State(std::move(state)) {}

bool CommandTicket::IsValid() const noexcept
{
    return static_cast<bool>(m_State);
}

bool CommandTicket::IsReady() const
{
    if (!m_State) return false;
    std::lock_guard lock(m_State->mutex);
    return m_State->result.status != CommandCompletionStatus::Pending;
}

std::uint64_t CommandTicket::Id() const noexcept
{
    return m_State ? m_State->id : 0;
}

TicketWaitStatus CommandTicket::Wait() const
{
    if (!m_State) return TicketWaitStatus::Invalid;
    if (m_State->worker && m_State->worker->IsCurrent() && !IsReady())
        return TicketWaitStatus::WouldDeadlock;
    std::unique_lock lock(m_State->mutex);
    m_State->condition.wait(lock, [&] { return m_State->result.status != CommandCompletionStatus::Pending; });
    return TicketWaitStatus::Completed;
}

TicketWaitStatus CommandTicket::WaitFor(std::chrono::milliseconds timeout) const
{
    if (!m_State) return TicketWaitStatus::Invalid;
    if (m_State->worker && m_State->worker->IsCurrent() && !IsReady())
        return TicketWaitStatus::WouldDeadlock;
    std::unique_lock lock(m_State->mutex);
    if (!m_State->condition.wait_for(
            lock, timeout, [&] { return m_State->result.status != CommandCompletionStatus::Pending; }))
        return TicketWaitStatus::Timeout;
    return TicketWaitStatus::Completed;
}

std::optional<CommandResult> CommandTicket::TryGetResult() const
{
    if (!m_State) return std::nullopt;
    std::lock_guard lock(m_State->mutex);
    if (m_State->result.status == CommandCompletionStatus::Pending) return std::nullopt;
    return m_State->result;
}

std::size_t RenderEnvelope::CommandCount() const noexcept
{
    return reliableCommands.size() + (drawCommand ? 1u : 0u);
}

std::size_t RenderEnvelope::PayloadBytes() const noexcept
{
    std::size_t result = additionalPayloadBytes;
    for (const auto& command : reliableCommands)
        if (command) result = AddSaturated(result, command->PayloadBytes());
    if (drawCommand) result = AddSaturated(result, drawCommand->PayloadBytes());
    return result;
}

struct RenderThread::Impl
{
    struct QueuedCommand
    {
        std::unique_ptr<IRenderCommand> command;
        std::shared_ptr<Detail::CommandTicketState> ticket;
    };

    struct QueuedEnvelope
    {
        CpuFrameId cpuFrameId = 0;
        std::vector<QueuedCommand> reliable;
        std::optional<QueuedCommand> draw;
        std::shared_ptr<Detail::CommandTicketState> ticket;
        std::size_t budgetCommandCount = 0;
        std::size_t payloadBytes = 0;
    };

    explicit Impl(RenderThreadLimits limitsValue, LifecycleCallback start, LifecycleCallback stop)
        : limits(limitsValue), onStart(std::move(start)), onStop(std::move(stop)),
          workerIdentity(std::make_shared<Detail::WorkerIdentity>()),
          ready(std::make_shared<Detail::CommandTicketState>())
    {
        ready->worker = workerIdentity;
    }

    std::shared_ptr<Detail::CommandTicketState> MakeTicketLocked()
    {
        auto ticket = std::make_shared<Detail::CommandTicketState>();
        ticket->id = nextTicket++;
        ticket->worker = workerIdentity;
        return ticket;
    }

    bool HasCapacityLocked(std::size_t commandCount, std::size_t payloadBytes, bool hasDraw) const
    {
        const std::size_t budgetCommands = std::max<std::size_t>(1, commandCount);
        if (hasDraw && drawOutstanding >= limits.maxDrawOutstanding) return false;
        if (budgetCommands > limits.maxCommandsOutstanding - commandsOutstanding) return false;
        if (payloadBytes > limits.maxPayloadBytes - payloadBytesOutstanding) return false;
        return true;
    }

    bool TooLarge(std::size_t commandCount, std::size_t payloadBytes, bool hasDraw) const
    {
        const std::size_t budgetCommands = std::max<std::size_t>(1, commandCount);
        return budgetCommands > limits.maxCommandsOutstanding || payloadBytes > limits.maxPayloadBytes ||
               (hasDraw && limits.maxDrawOutstanding == 0);
    }

    void ReleaseBudgetLocked(const QueuedEnvelope& envelope)
    {
        commandsOutstanding -= envelope.budgetCommandCount;
        payloadBytesOutstanding -= envelope.payloadBytes;
        if (envelope.draw) --drawOutstanding;
        processingEnvelope = false;
        capacityCondition.notify_all();
    }

    void CancelCommand(QueuedCommand& command, CommandCompletionStatus status, const std::string& message)
    {
        command.ticket->Complete(status, message);
        command.command.reset();
    }

    void CancelQueueLocked(CommandCompletionStatus status, const std::string& message)
    {
        while (!queue.empty())
        {
            auto envelope = std::move(queue.front());
            queue.pop_front();
            for (auto& command : envelope.reliable) CancelCommand(command, status, message);
            if (envelope.draw) CancelCommand(*envelope.draw, status, message);
            envelope.ticket->Complete(status, message);
            commandsOutstanding -= envelope.budgetCommandCount;
            payloadBytesOutstanding -= envelope.payloadBytes;
            if (envelope.draw) --drawOutstanding;
        }
        capacityCondition.notify_all();
    }

    void Fail(std::string message)
    {
        std::lock_guard lock(mutex);
        state = RenderThreadState::Failed;
        failureMessage = std::move(message);
        ready->Complete(CommandCompletionStatus::Failed, failureMessage);
        CancelQueueLocked(CommandCompletionStatus::Cancelled, failureMessage);
        queueCondition.notify_all();
        capacityCondition.notify_all();
    }

    void WorkerMain()
    {
        {
            std::lock_guard identityLock(workerIdentity->mutex);
            workerIdentity->id = std::this_thread::get_id();
        }

        try
        {
            if (onStart) onStart();
            {
                std::lock_guard lock(mutex);
                if (state == RenderThreadState::Starting)
                    state = RenderThreadState::Running;
                ready->Complete(state == RenderThreadState::Running ? CommandCompletionStatus::Succeeded
                                                                    : CommandCompletionStatus::Cancelled,
                                state == RenderThreadState::Running ? std::string{} : "stopped during startup");
                capacityCondition.notify_all();
            }

            for (;;)
            {
                QueuedEnvelope envelope;
                {
                    std::unique_lock lock(mutex);
                    queueCondition.wait(lock, [&] {
                        return !queue.empty() || state == RenderThreadState::Stopping ||
                               state == RenderThreadState::Failed;
                    });
                    if (state == RenderThreadState::Failed) break;
                    if (queue.empty())
                    {
                        if (state == RenderThreadState::Stopping) break;
                        continue;
                    }
                    envelope = std::move(queue.front());
                    queue.pop_front();
                    processingEnvelope = true;
                }

                RenderFrameContext context{};
                context.cpuFrameId = envelope.cpuFrameId;
                bool failed = false;
                bool drawCancelled = false;
                std::string error;
                std::size_t reliableIndex = 0;
                for (; reliableIndex < envelope.reliable.size(); ++reliableIndex)
                {
                    auto& item = envelope.reliable[reliableIndex];
                    try
                    {
                        item.command->Execute(context);
                        item.ticket->Complete(CommandCompletionStatus::Succeeded);
                    }
                    catch (...)
                    {
                        error = CurrentExceptionMessage();
                        item.ticket->Complete(CommandCompletionStatus::Failed, error);
                        failed = true;
                        ++reliableIndex;
                        break;
                    }
                }
                for (; reliableIndex < envelope.reliable.size(); ++reliableIndex)
                    CancelCommand(envelope.reliable[reliableIndex], CommandCompletionStatus::Cancelled,
                                  failed ? error : "render command cancelled");

                if (envelope.draw)
                {
                    bool stopping = false;
                    {
                        std::lock_guard lock(mutex);
                        stopping = state == RenderThreadState::Stopping;
                    }
                    if (failed || stopping)
                    {
                        drawCancelled = true;
                        CancelCommand(*envelope.draw, CommandCompletionStatus::Cancelled,
                                      failed ? error : "draw cancelled during shutdown");
                    }
                    else
                    {
                        try
                        {
                            envelope.draw->command->Execute(context);
                            envelope.draw->ticket->Complete(CommandCompletionStatus::Succeeded);
                        }
                        catch (...)
                        {
                            error = CurrentExceptionMessage();
                            envelope.draw->ticket->Complete(CommandCompletionStatus::Failed, error);
                            failed = true;
                        }
                    }
                }

                envelope.ticket->Complete(failed ? CommandCompletionStatus::Failed
                                                 : drawCancelled ? CommandCompletionStatus::Cancelled
                                                                 : CommandCompletionStatus::Succeeded,
                                          error);
                {
                    std::lock_guard lock(mutex);
                    ReleaseBudgetLocked(envelope);
                }
                if (failed)
                {
                    Fail(error);
                    break;
                }
            }
        }
        catch (...)
        {
            Fail(CurrentExceptionMessage());
        }

        try
        {
            if (onStop) onStop();
        }
        catch (...)
        {
            Fail(CurrentExceptionMessage());
        }

        {
            std::lock_guard lock(mutex);
            if (state != RenderThreadState::Failed) state = RenderThreadState::Stopped;
            CancelQueueLocked(CommandCompletionStatus::Cancelled, "render thread stopped");
            ready->Complete(CommandCompletionStatus::Cancelled, "render thread stopped before becoming ready");
            capacityCondition.notify_all();
        }
        {
            std::lock_guard identityLock(workerIdentity->mutex);
            workerIdentity->id = {};
        }
    }

    RenderThreadLimits limits;
    LifecycleCallback onStart;
    LifecycleCallback onStop;
    std::shared_ptr<Detail::WorkerIdentity> workerIdentity;
    std::shared_ptr<Detail::CommandTicketState> ready;
    mutable std::mutex mutex;
    std::condition_variable queueCondition;
    std::condition_variable capacityCondition;
    std::deque<QueuedEnvelope> queue;
    std::thread worker;
    RenderThreadState state = RenderThreadState::Stopped;
    bool started = false;
    bool processingEnvelope = false;
    std::uint64_t nextTicket = 1;
    std::size_t drawOutstanding = 0;
    std::size_t commandsOutstanding = 0;
    std::size_t payloadBytesOutstanding = 0;
    std::string failureMessage;
};

RenderThread::RenderThread(RenderThreadLimits limits, LifecycleCallback onStart, LifecycleCallback onStop)
    : m_Impl(std::make_unique<Impl>(limits, std::move(onStart), std::move(onStop)))
{
}

RenderThread::~RenderThread()
{
    RequestStop();
    Join();
}

bool RenderThread::Start()
{
    std::lock_guard lock(m_Impl->mutex);
    if (m_Impl->started) return false;
    m_Impl->started = true;
    m_Impl->state = RenderThreadState::Starting;
    m_Impl->worker = std::thread([impl = m_Impl.get()] { impl->WorkerMain(); });
    return true;
}

CommandTicket RenderThread::ReadyTicket() const
{
    return CommandTicket(m_Impl->ready);
}

RenderSubmitResult RenderThread::TrySubmit(RenderEnvelope& source)
{
    const std::size_t commandCount = source.CommandCount();
    const std::size_t payloadBytes = source.PayloadBytes();
    const bool hasDraw = source.HasDraw();
    std::lock_guard lock(m_Impl->mutex);
    if (m_Impl->state == RenderThreadState::Failed)
        return {RenderSubmitStatus::Failed, {}, m_Impl->failureMessage};
    if (m_Impl->state == RenderThreadState::Stopped || m_Impl->state == RenderThreadState::Stopping)
        return {RenderSubmitStatus::Stopped, {}, "render thread is stopped"};
    if (m_Impl->state != RenderThreadState::Running)
        return {RenderSubmitStatus::NotReady, {}, "render thread is not ready"};
    if (std::ranges::any_of(source.reliableCommands, [](const auto& command) { return !command; }))
        return {RenderSubmitStatus::Invalid, {}, "envelope contains a null render command"};
    if (m_Impl->TooLarge(commandCount, payloadBytes, hasDraw))
        return {RenderSubmitStatus::TooLarge, {}, "envelope exceeds render queue limits"};
    if (!m_Impl->HasCapacityLocked(commandCount, payloadBytes, hasDraw))
        return {RenderSubmitStatus::Full, {}, "render queue is full"};

    Impl::QueuedEnvelope queued;
    queued.cpuFrameId = source.cpuFrameId;
    queued.payloadBytes = payloadBytes;
    queued.budgetCommandCount = std::max<std::size_t>(1, commandCount);
    queued.ticket = m_Impl->MakeTicketLocked();

    RenderSubmitReceipt receipt;
    receipt.envelope = CommandTicket(queued.ticket);
    queued.reliable.reserve(source.reliableCommands.size());
    receipt.reliableCommands.reserve(source.reliableCommands.size());
    for (auto& command : source.reliableCommands)
    {
        auto ticket = m_Impl->MakeTicketLocked();
        receipt.reliableCommands.emplace_back(CommandTicket(ticket));
        queued.reliable.push_back({std::move(command), std::move(ticket)});
    }
    source.reliableCommands.clear();
    if (source.drawCommand)
    {
        auto ticket = m_Impl->MakeTicketLocked();
        receipt.draw.emplace(CommandTicket(ticket));
        queued.draw.emplace(Impl::QueuedCommand{std::move(source.drawCommand), std::move(ticket)});
        ++m_Impl->drawOutstanding;
    }

    m_Impl->commandsOutstanding += queued.budgetCommandCount;
    m_Impl->payloadBytesOutstanding += queued.payloadBytes;
    m_Impl->queue.push_back(std::move(queued));
    m_Impl->queueCondition.notify_one();
    return {RenderSubmitStatus::Accepted, std::move(receipt), {}};
}

bool RenderThread::WaitForCapacity(std::size_t commandCount, std::size_t payloadBytes, bool hasDraw,
                                   std::chrono::milliseconds timeout)
{
    std::unique_lock lock(m_Impl->mutex);
    if (m_Impl->TooLarge(commandCount, payloadBytes, hasDraw)) return false;
    return m_Impl->capacityCondition.wait_for(lock, timeout, [&] {
        return m_Impl->state != RenderThreadState::Running ||
               m_Impl->HasCapacityLocked(commandCount, payloadBytes, hasDraw);
    }) && m_Impl->state == RenderThreadState::Running &&
           m_Impl->HasCapacityLocked(commandCount, payloadBytes, hasDraw);
}

TicketWaitStatus RenderThread::FlushCpuThrough(const CommandTicket& ticket) const
{
    return ticket.Wait();
}

void RenderThread::RequestStop()
{
    std::lock_guard lock(m_Impl->mutex);
    if (!m_Impl->started)
    {
        m_Impl->started = true;
        m_Impl->ready->Complete(CommandCompletionStatus::Cancelled, "render thread was never started");
        return;
    }
    if (m_Impl->state == RenderThreadState::Starting || m_Impl->state == RenderThreadState::Running)
        m_Impl->state = RenderThreadState::Stopping;
    m_Impl->queueCondition.notify_all();
    m_Impl->capacityCondition.notify_all();
}

void RenderThread::Join()
{
    RequestStop();
    if (m_Impl->worker.joinable() && !IsRenderThread()) m_Impl->worker.join();
}

RenderThreadState RenderThread::State() const noexcept
{
    std::lock_guard lock(m_Impl->mutex);
    return m_Impl->state;
}

RenderThreadStats RenderThread::Stats() const
{
    std::lock_guard lock(m_Impl->mutex);
    return {m_Impl->drawOutstanding, m_Impl->commandsOutstanding, m_Impl->payloadBytesOutstanding,
            m_Impl->queue.size(), m_Impl->processingEnvelope};
}

bool RenderThread::IsRenderThread() const
{
    return m_Impl->workerIdentity->IsCurrent();
}
} // namespace Aether::Render
