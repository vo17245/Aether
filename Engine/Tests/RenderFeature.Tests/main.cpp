#include <Render/Feature/RenderFeature.h>
#include <Render/Threads/RenderThread.h>
#include <Render/RenderGraph/Resource/ResourceArena.h>
#include <Render/RenderGraph/Resource/ResourceLruPool.h>

#include <cstdint>
#include <memory>
#include <semaphore>
#include <string>
#include <vector>

namespace
{
int failures = 0;

void Check(bool condition)
{
    if (!condition)
        ++failures;
}

struct Snapshot final : Aether::Render::RenderFeatureData
{
    explicit Snapshot(std::vector<std::uint32_t> values) : values(std::move(values)) {}
    std::size_t PayloadBytes() const noexcept override { return values.size() * sizeof(values[0]); }
    std::vector<std::uint32_t> values;
};

struct Observation
{
    std::vector<std::uint32_t> values;
    Aether::Render::CpuFrameId cpuFrameId = 0;
    std::uint32_t frameSlot = 0;
    int featureDestructions = 0;
};

class Feature final : public Aether::Render::RenderFeature
{
public:
    explicit Feature(std::shared_ptr<Observation> observation) : m_Observation(std::move(observation)) {}
    ~Feature() override { ++m_Observation->featureDestructions; }

    void PrepareFrame(Aether::Render::RenderFrameContext& context,
                      const Aether::Render::RenderFeatureData& frameData) override
    {
        const auto* snapshot = dynamic_cast<const Snapshot*>(&frameData);
        Check(snapshot != nullptr);
        if (!snapshot)
            return;
        m_Observation->values = snapshot->values;
        m_Observation->cpuFrameId = context.cpuFrameId.value_or(0);
        m_Observation->frameSlot = context.frameSlot;
    }

private:
    std::shared_ptr<Observation> m_Observation;
};
} // namespace

int main()
{
    auto observation = std::make_shared<Observation>();
    std::weak_ptr<Aether::Render::RenderFeature> weakFeature;
    {
        Aether::Render::RenderFeatureFrame frame(42);
        auto feature = std::make_shared<Feature>(observation);
        weakFeature = feature;
        std::vector<std::uint32_t> frontendValues{1, 2, 3, 5};
        frame.Emplace<Snapshot>(feature, frontendValues);
        Check(frame.Size() == 1);
        Check(frame.PayloadBytes() == frontendValues.size() * sizeof(frontendValues[0]));

        frontendValues.assign({99});
        feature.reset();
        Check(!weakFeature.expired());

        Aether::Render::RenderFrameContext context{.frameSlot = 1};
        frame.Prepare(context);
        Check((observation->values == std::vector<std::uint32_t>{1, 2, 3, 5}));
        Check(observation->cpuFrameId == 42);
        Check(observation->frameSlot == 1);
    }
    Check(weakFeature.expired());
    Check(observation->featureDestructions == 1);

    // An accepted frame owns both its feature and immutable snapshot even when
    // the frontend releases them before the render worker reaches that frame.
    {
        using namespace Aether::Render;
        RenderThread thread;
        Check(thread.Start());
        Check(thread.ReadyTicket().Wait() == TicketWaitStatus::Completed);
        std::binary_semaphore entered{0};
        std::binary_semaphore release{0};
        RenderEnvelope blocker;
        blocker.drawCommand = MakeRenderCommand([&] {
            entered.release();
            release.acquire();
        });
        auto blocked = thread.TrySubmit(blocker);
        Check(blocked.status == RenderSubmitStatus::Accepted);
        entered.acquire();

        auto queuedObservation = std::make_shared<Observation>();
        auto queuedFeature = std::make_shared<Feature>(queuedObservation);
        std::weak_ptr<RenderFeature> queuedWeak = queuedFeature;
        RenderFeatureFrame queuedFrame(77);
        queuedFrame.Emplace<Snapshot>(queuedFeature, std::vector<std::uint32_t>{8, 13, 21});
        RenderEnvelope queued;
        queued.drawCommand = MakeRenderCommand(
            [frame = std::move(queuedFrame)](RenderFrameContext& context) mutable {
                context.frameSlot = 1;
                frame.Prepare(context);
            });
        auto accepted = thread.TrySubmit(queued);
        Check(accepted.status == RenderSubmitStatus::Accepted);
        queuedFeature.reset();
        Check(!queuedWeak.expired());

        release.release();
        Check(accepted.receipt.envelope.Wait() == TicketWaitStatus::Completed);
        const auto result = accepted.receipt.envelope.TryGetResult();
        Check(result && result->status == CommandCompletionStatus::Succeeded);
        Check((queuedObservation->values == std::vector<std::uint32_t>{8, 13, 21}));
        Check(queuedObservation->cpuFrameId == 77 && queuedObservation->frameSlot == 1);
        RenderEnvelope drained;
        drained.reliableCommands.push_back(MakeRenderCommand([] {}));
        auto drainResult = thread.TrySubmit(drained);
        Check(drainResult.status == RenderSubmitStatus::Accepted);
        Check(drainResult.receipt.envelope.Wait() == TicketWaitStatus::Completed);
        Check(queuedWeak.expired());
        thread.RequestStop();
        thread.Join();
    }

    std::weak_ptr<int> weakLifetime;
    {
        Aether::RenderGraph::ResourceArena arena;
        Aether::RenderGraph::ResourceLruPool pool(&arena);
        Aether::RenderGraph::RenderGraph graph(&arena, &pool);
        auto lifetime = std::make_shared<int>(7);
        weakLifetime = lifetime;
        graph.RetainLifetime(lifetime);
        lifetime.reset();
        Check(!weakLifetime.expired());
    }
    Check(weakLifetime.expired());

    return failures == 0 ? 0 : 1;
}
