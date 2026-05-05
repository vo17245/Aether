#include <Render/RHI/DescriptorSet.h>
#include <Render/RHI/Backend/Vulkan/GlobalRenderContext.h>
#include <Render/Config.h>
#include <Core/Core.h>
namespace Aether::rhi
{
DescriptorSet DescriptorSet::Create(uint32_t samplerCount, uint32_t uboCount, uint32_t ssboCount)
{
    if (Render::Config::RenderApi == Render::Api::Vulkan)
    {
        auto setOpt = vk::GRC::GetCurrentFrameDynamicDescriptorPool().CreateSet(uboCount, samplerCount, ssboCount);
        if (!setOpt)
        {
            assert(false && "Failed to create descriptor set");
            return {};
        }
        return DescriptorSet(std::move(*setOpt));
    }
    else
    {
        assert(false && "Unsupported Render API");
        return {};
    }
}
} // namespace Aether::rhi