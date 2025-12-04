#include "ShaderModule.h"
#include "GlobalRenderContext.h"
#include "Filesystem/Utils.h"
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {

std::optional<ShaderModule> ShaderModule::CreateFromBinaryCode(const std::span<uint32_t> code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size()*4;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(GRC::GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        assert(false&&"failed to create shader module");
    }

    return ShaderModule(shaderModule);
}
Scope<ShaderModule> ShaderModule::CreateScopeFromBinaryCode(const std::span<uint32_t> code)
{
    assert(!code.empty());
    auto opt = CreateFromBinaryCode(code);
    if (!opt)
    {
        return nullptr;
    }
    return CreateScope<ShaderModule>(std::move(opt.value()));
}
std::optional<ShaderModule> ShaderModule::CreateFromBinaryFile(const std::string& filepath)
{
    auto code = Filesystem::ReadFile(filepath);
    
    if (!code)
    {
        return std::nullopt;
    }
    if(code->size()%4!=0)
    {
        return std::nullopt;
    }
    return CreateFromBinaryCode(std::span<uint32_t>((uint32_t*)code->data(),code->size()/4));
}
ShaderModule::~ShaderModule()
{
    if (m_ShaderModule != VK_NULL_HANDLE)
        vkDestroyShaderModule(GRC::GetDevice(), m_ShaderModule, nullptr);
}
VkShaderModule ShaderModule::GetHandle() const
{
    return m_ShaderModule;
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept
{
    m_ShaderModule = other.m_ShaderModule;
    other.m_ShaderModule = VK_NULL_HANDLE;
}
ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (m_ShaderModule != VK_NULL_HANDLE)
        vkDestroyShaderModule(GRC::GetDevice(), m_ShaderModule, nullptr);
    m_ShaderModule = other.m_ShaderModule;
    other.m_ShaderModule = VK_NULL_HANDLE;
    return *this;
}
void ShaderModule::SetEntryPoint(const std::string& entryPoint)
{
    m_EntryPoint = entryPoint;
}
const std::string& ShaderModule::GetEntryPoint() const
{
    return m_EntryPoint;
}

ShaderModule::ShaderModule(VkShaderModule shaderModule) :
    m_ShaderModule(shaderModule)
{
}

}
} // namespace Aether::vk