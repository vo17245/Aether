#include "RuntimeRegistration.h"

#include <Entry/Application.h>

#include <stdexcept>

namespace
{
class SampleRuntimeApplication final : public Aether::Application
{
public:
    void OnInit(Aether::Window&) override
    {
        auto registry = SampleGame::BuildRuntimeRegistry(SampleGame::MakeProjectManifest());
        if (!registry) throw std::runtime_error(registry.error().message);
        m_Runtime = std::move(*registry);
    }
    const char* GetName() const override { return "Sample Game"; }
private:
    std::shared_ptr<const Aether::GameFeatures::RuntimeRegistry> m_Runtime;
};
}

DEFINE_APPLICATION(SampleRuntimeApplication)
