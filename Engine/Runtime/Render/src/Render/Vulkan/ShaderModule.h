#pragma once

#include "Filesystem/Utils.h"
#include "vulkan/vulkan_core.h"
#include <span>
namespace Aether {
namespace vk {

class ShaderModule
{
public:
    static std::optional<ShaderModule> CreateFromBinaryCode(const std::span<uint32_t> code);
    static Scope<ShaderModule> CreateScopeFromBinaryCode(const std::span<uint32_t> code);
    static std::optional<ShaderModule> CreateFromBinaryFile(const std::string& filepath);
    ~ShaderModule();
    VkShaderModule GetHandle() const;
    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&& other) noexcept;
    ShaderModule& operator=(ShaderModule&& other) noexcept;
    void SetEntryPoint(const std::string& entryPoint);
    const std::string& GetEntryPoint() const;

private:
    ShaderModule(VkShaderModule shaderModule);
    VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
    std::string m_EntryPoint = "main";
};
}
} // namespace Aether::vk