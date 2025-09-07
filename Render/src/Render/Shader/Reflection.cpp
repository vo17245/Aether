#include "Reflection.h"
#include <spirv_cross/spirv_hlsl.hpp>
#include <Debug/Log.h>
namespace Aether
{
static std::string BaseTypeToString(spirv_cross::SPIRType::BaseType type)
{
    switch (type)
    {
    case spirv_cross::SPIRType::BaseType::Float:
        return "Float";
    default:
        return "NotImplemented For This Type";
    }
}
std::expected<ShaderParameterMap, std::string> ReflectShaderParameters(const std::vector<uint32_t>& spirv)
{
    // 创建 GLSL 编译器（也可 HLSL/MSL）
    spirv_cross::CompilerHLSL compiler(spirv);

    // 获取资源
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    // 遍历 Uniform Buffers
    for (auto& ubo : resources.uniform_buffers)
    {
        Debug::Log::Debug("UBO Name: {}", ubo.name);
        // 获取 buffer 类型
        auto& type = compiler.get_type(ubo.base_type_id);

        // 大小（字节）
        size_t size = compiler.get_declared_struct_size(type);
        Debug::Log::Debug("  Size: {} bytes", size);

        // 成员信息
        for (uint32_t i = 0; i < type.member_types.size(); ++i)
        {
            std::string member_name = compiler.get_member_name(ubo.base_type_id, i);
            uint32_t offset = compiler.type_struct_member_offset(type, i);
            auto& member_type = compiler.get_type(type.member_types[i]);
            if (member_type.basetype == spirv_cross::SPIRType::Struct)
            {
                Debug::Log::Debug("  Member: {}, Offset: {}, Type: Struct, Declared Struct Size: {}", member_name,
                                  offset, compiler.get_declared_struct_size(member_type));
            }
            else
            {
                Debug::Log::Debug("  Member: {}, Offset: {}, Type: {}, VecSize: {}, Columns: {}", member_name, offset,
                                  BaseTypeToString(member_type.basetype),member_type.vecsize,member_type.columns);
            }
        }
    }

    // 遍历纹理 / sampler
    for (auto& tex : resources.sampled_images)
    {
        Debug::Log::Debug("Texture: {}, Binding: {}, Set: {}", tex.name,
                          compiler.get_decoration(tex.id, spv::DecorationBinding),
                          compiler.get_decoration(tex.id, spv::DecorationDescriptorSet));
    }

    return std::unexpected<std::string>("Not Implemented");
}
} // namespace Aether