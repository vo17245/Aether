#include <Render/Threads/RenderThread.h>
#include <Render/RHI/Backend/Vulkan/GlobalRenderContext.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::chrono_literals;
using namespace Aether::Render;

namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

CommandResult WaitResult(const CommandTicket& ticket)
{
    Check(ticket.WaitFor(2s) == TicketWaitStatus::Completed, "ticket did not complete");
    auto result = ticket.TryGetResult();
    Check(result.has_value(), "completed ticket has no result");
    return *result;
}

void WaitReady(RenderThread& thread)
{
    Check(thread.Start(), "render thread did not start");
    Check(WaitResult(thread.ReadyTicket()).status == CommandCompletionStatus::Succeeded,
          "render thread did not become ready");
}

void TestFifoAndOwnership()
{
    RenderThread thread;
    WaitReady(thread);
    std::vector<int> order;
    for (int value = 1; value <= 3; ++value)
    {
        RenderEnvelope envelope;
        envelope.reliableCommands.push_back(MakeRenderCommand([&, value] { order.push_back(value); }));
        auto result = thread.TrySubmit(envelope);
        Check(result.status == RenderSubmitStatus::Accepted, "FIFO command was rejected");
        Check(envelope.reliableCommands.empty(), "accepted command ownership was not transferred");
        Check(WaitResult(result.receipt.envelope).status == CommandCompletionStatus::Succeeded,
              "FIFO envelope failed");
    }
    Check(order == std::vector<int>({1, 2, 3}), "commands were not executed FIFO");
    thread.RequestStop();
    thread.Join();
    Check(thread.State() == RenderThreadState::Stopped, "thread did not stop");
    thread.RequestStop();
    thread.Join();
}

void TestBackpressureAndStopDrain()
{
    RenderThread thread({.maxDrawOutstanding = 2, .maxCommandsOutstanding = 8, .maxPayloadBytes = 32});
    WaitReady(thread);
    std::binary_semaphore entered{0};
    std::binary_semaphore release{0};
    std::atomic<int> reliableCount = 0;

    RenderEnvelope first;
    first.drawCommand = MakeRenderCommand([&] {
        entered.release();
        release.acquire();
    }, 8);
    auto firstResult = thread.TrySubmit(first);
    Check(firstResult.status == RenderSubmitStatus::Accepted, "first draw rejected");
    entered.acquire();
    auto stats = thread.Stats();
    Check(stats.drawOutstanding == 1 && stats.processingEnvelope, "running draw did not retain credit");

    RenderEnvelope second;
    second.reliableCommands.push_back(MakeRenderCommand([&] { ++reliableCount; }));
    second.drawCommand = MakeRenderCommand([] {}, 8);
    auto secondResult = thread.TrySubmit(second);
    Check(secondResult.status == RenderSubmitStatus::Accepted, "second draw rejected");

    RenderEnvelope full;
    full.drawCommand = MakeRenderCommand([] {});
    auto fullResult = thread.TrySubmit(full);
    Check(fullResult.status == RenderSubmitStatus::Full, "draw credit did not apply backpressure");
    Check(full.drawCommand != nullptr, "rejected envelope lost ownership");

    RenderEnvelope tooLarge;
    tooLarge.reliableCommands.push_back(MakeRenderCommand([] {}, 33));
    auto tooLargeResult = thread.TrySubmit(tooLarge);
    Check(tooLargeResult.status == RenderSubmitStatus::TooLarge, "oversized command was not rejected");
    Check(tooLarge.reliableCommands.front() != nullptr, "oversized envelope lost ownership");

    thread.RequestStop();
    release.release();
    thread.Join();
    Check(reliableCount == 1, "reliable command was discarded during stop");
    Check(WaitResult(secondResult.receipt.reliableCommands.front()).status == CommandCompletionStatus::Succeeded,
          "reliable command did not succeed during stop");
    Check(WaitResult(*secondResult.receipt.draw).status == CommandCompletionStatus::Cancelled,
          "queued draw was not cancelled during stop");
    stats = thread.Stats();
    Check(stats.drawOutstanding == 0 && stats.commandsOutstanding == 0 && stats.payloadBytesOutstanding == 0,
          "queue budgets were not returned");
}

void TestFailureAndSelfWait()
{
    RenderThread thread;
    WaitReady(thread);
    std::binary_semaphore entered{0};
    std::binary_semaphore release{0};
    CommandTicket later;
    std::atomic<TicketWaitStatus> selfWait = TicketWaitStatus::Invalid;

    RenderEnvelope envelope;
    envelope.reliableCommands.push_back(MakeRenderCommand([&] {
        entered.release();
        release.acquire();
    }));
    envelope.drawCommand = MakeRenderCommand([&] { selfWait = later.Wait(); });
    auto result = thread.TrySubmit(envelope);
    Check(result.status == RenderSubmitStatus::Accepted, "self-wait envelope rejected");
    later = result.receipt.envelope;
    entered.acquire();
    release.release();
    Check(WaitResult(result.receipt.envelope).status == CommandCompletionStatus::Succeeded,
          "self-wait envelope did not finish");
    Check(selfWait == TicketWaitStatus::WouldDeadlock, "render-thread self wait was not rejected");

    RenderEnvelope failing;
    failing.reliableCommands.push_back(MakeRenderCommand([] { throw std::runtime_error("expected failure"); }));
    auto failed = thread.TrySubmit(failing);
    Check(failed.status == RenderSubmitStatus::Accepted, "failing command was rejected before execution");
    const auto failure = WaitResult(failed.receipt.envelope);
    Check(failure.status == CommandCompletionStatus::Failed && failure.message == "expected failure",
          "command exception was not preserved");
    thread.Join();
    Check(thread.State() == RenderThreadState::Failed, "command exception did not fail renderer");
}

void TestStartupFailure()
{
    RenderThread thread({}, [] { throw std::runtime_error("startup failure"); });
    Check(thread.Start(), "startup failure thread did not start");
    const auto ready = WaitResult(thread.ReadyTicket());
    Check(ready.status == CommandCompletionStatus::Failed && ready.message == "startup failure",
          "startup failure was not returned");
    thread.Join();
    Check(thread.State() == RenderThreadState::Failed, "startup failure did not set failed state");
}

void TestInvalidAndNeverStartedStop()
{
    RenderThread thread;
    thread.RequestStop();
    Check(!thread.Start(), "thread restarted after being stopped before startup");
    Check(WaitResult(thread.ReadyTicket()).status == CommandCompletionStatus::Cancelled,
          "never-started ready ticket was not cancelled");

    RenderEnvelope stoppedEnvelope;
    stoppedEnvelope.reliableCommands.push_back(nullptr);
    Check(thread.TrySubmit(stoppedEnvelope).status == RenderSubmitStatus::Stopped,
          "stopped state was not reported before payload validation");

    RenderThread running;
    WaitReady(running);
    RenderEnvelope invalid;
    invalid.reliableCommands.push_back(nullptr);
    Check(running.TrySubmit(invalid).status == RenderSubmitStatus::Invalid,
          "null command was accepted");
    Check(invalid.reliableCommands.size() == 1, "invalid envelope lost ownership");
    running.RequestStop();
    running.Join();
}

void TestRuntimeGpuOwnerGuard()
{
    Aether::Render::RenderThread thread(
        {}, [] { Aether::vk::GRC::BindRuntimeRenderThread(); },
        [] { Aether::vk::GRC::UnbindRuntimeRenderThread(); });
    WaitReady(thread);

    bool rejectedMainThread = false;
    try
    {
        Aether::vk::GRC::AssertRuntimeRenderThread();
    }
    catch (const std::logic_error&)
    {
        rejectedMainThread = true;
    }
    Check(rejectedMainThread, "runtime GPU owner guard accepted the main thread");

    RenderEnvelope envelope;
    envelope.reliableCommands.push_back(MakeRenderCommand(
        [] { Aether::vk::GRC::AssertRuntimeRenderThread(); }));
    auto result = thread.TrySubmit(envelope);
    Check(result.status == RenderSubmitStatus::Accepted, "owner guard command was rejected");
    Check(WaitResult(result.receipt.envelope).status == CommandCompletionStatus::Succeeded,
          "owner guard rejected RenderThread");
    thread.RequestStop();
    thread.Join();

    // Bootstrap/final teardown are explicitly permitted once ownership is released.
    Aether::vk::GRC::AssertRuntimeRenderThread();
}
} // namespace

int main()
{
    try
    {
        TestFifoAndOwnership();
        TestBackpressureAndStopDrain();
        TestFailureAndSelfWait();
        TestStartupFailure();
        TestInvalidAndNeverStartedStop();
        TestRuntimeGpuOwnerGuard();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
    return 0;
}
