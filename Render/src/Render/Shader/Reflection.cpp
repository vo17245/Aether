#include "Reflection.h"
#include <spirv_cross/spirv_hlsl.hpp>
#include <Debug/Log.h>
namespace Aether
{
class ShaderParameterMapBuilder
{
public:
    ShaderParameterMapBuilder(ShaderParameterMap& map)
        : m_Map(map)
    {
    };
    void BeginUniformBuffer(const std::string& name,uint16_t sizeInBytes,uint16_t set,uint16_t binding)
    {
        assert(!m_InBuffer && "Already in a buffer");
        ShaderParameterMap::BufferInfo info;
        info.name = name;
        info.size=sizeInBytes;
        info.allocationBegin=m_Map.m_Allocations.size();
        info.allocationCount=0;
        info.set=set;
        info.binding=binding;
        m_Map.m_UniformBuffers.emplace_back(std::move(info));
        m_InBuffer = true;

    }
    void PushAllocation(const std::string& name, const ShaderParameterMap::ParameterAllocation& allocation)
    {
        assert(!m_Map.m_ParameterMap.contains(name) && "Parameter already exists");
        m_Map.m_ParameterMap[name] = (uint16_t)m_Map.m_Allocations.size();
        m_Map.m_Allocations.push_back(allocation);
        assert(!m_Map.m_UniformBuffers.empty() && "No buffer to push allocation to");
        m_Map.m_UniformBuffers.back().size++;
    }
    void EndUniformBuffer()
    {
        assert(m_InBuffer && "Not in a buffer");
        m_InBuffer = false;
    }
    uint16_t CurrentBufferIndex() const
    {
        assert(m_InBuffer && "Not in a buffer");
        return (uint16_t)(m_Map.m_UniformBuffers.size() - 1);
    }
    void PushSampler(const std::string& name,uint16_t set,uint16_t binding)
    {
        assert(!m_Map.m_ParameterMap.contains(name) && "Parameter already exists");
        m_Map.m_ParameterMap[name] = (uint16_t)m_Map.m_Samplers.size();
        ShaderParameterMap::SamplerInfo info;
        info.name=name;
        m_Map.m_Samplers.push_back(info);
    }
private:
    ShaderParameterMap& m_Map;
    bool m_InBuffer = false;
};
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
static ShaderParameterMap::BaseType ConvertBaseType(spirv_cross::SPIRType::BaseType type)
{
    switch (type)
    {
    case spirv_cross::SPIRType::BaseType::Unknown:
        return ShaderParameterMap::BaseType::Unknown;
    case spirv_cross::SPIRType::BaseType::Void:
        return ShaderParameterMap::BaseType::Void;
    case spirv_cross::SPIRType::BaseType::Boolean:
        return ShaderParameterMap::BaseType::Boolean;
    case spirv_cross::SPIRType::BaseType::SByte:
        return ShaderParameterMap::BaseType::SByte;
    case spirv_cross::SPIRType::BaseType::UByte:
        return ShaderParameterMap::BaseType::UByte;
    case spirv_cross::SPIRType::BaseType::Short:
        return ShaderParameterMap::BaseType::Short;
    case spirv_cross::SPIRType::BaseType::UShort:
        return ShaderParameterMap::BaseType::UShort;
    case spirv_cross::SPIRType::BaseType::Int:
        return ShaderParameterMap::BaseType::Int;
    case spirv_cross::SPIRType::BaseType::UInt:
        return ShaderParameterMap::BaseType::UInt;
    case spirv_cross::SPIRType::BaseType::Int64:
        return ShaderParameterMap::BaseType::Int64;
    case spirv_cross::SPIRType::BaseType::UInt64:
        return ShaderParameterMap::BaseType::UInt64;
    case spirv_cross::SPIRType::BaseType::AtomicCounter:
        return ShaderParameterMap::BaseType::AtomicCounter;
    case spirv_cross::SPIRType::BaseType::Half:
        return ShaderParameterMap::BaseType::Half;
    case spirv_cross::SPIRType::BaseType::Float:
        return ShaderParameterMap::BaseType::Float;
    case spirv_cross::SPIRType::BaseType::Double:
        return ShaderParameterMap::BaseType::Double;
    case spirv_cross::SPIRType::BaseType::Struct:
        return ShaderParameterMap::BaseType::Struct;
    case spirv_cross::SPIRType::BaseType::Image:
        return ShaderParameterMap::BaseType::Image;
    case spirv_cross::SPIRType::BaseType::SampledImage:
        return ShaderParameterMap::BaseType::SampledImage;
    case spirv_cross::SPIRType::BaseType::Sampler:
        return ShaderParameterMap::BaseType::Sampler;
    case spirv_cross::SPIRType::BaseType::AccelerationStructure:
        return ShaderParameterMap::BaseType::AccelerationStructure;
    case spirv_cross::SPIRType::BaseType::RayQuery:
        return ShaderParameterMap::BaseType::RayQuery;
    case spirv_cross::SPIRType::BaseType::CoopVecNV:
        return ShaderParameterMap::BaseType::CoopVecNV;
    default:
        assert(false && "Not Implemented For This Type");
        return ShaderParameterMap::BaseType::Unknown;
    }
}
static ShaderParameterMap::ParameterAllocation CreateAllocation(uint16_t bufferIndex, uint16_t offset,
                                                                ShaderParameterMap::BaseType baseType, uint8_t vecSize,
                                                                uint8_t columns)

{
    ShaderParameterMap::ParameterAllocation allocation;
    allocation.bufferIndex = bufferIndex;
    allocation.offset = offset;
    allocation.type = baseType;
    allocation.vecSize = vecSize;
    allocation.columns = columns;

    return allocation;
}
std::expected<ShaderParameterMap, std::string> ReflectShaderParameters(const std::vector<uint32_t>& spirv)
{
    // 创建 GLSL 编译器（也可 HLSL/MSL）
    spirv_cross::CompilerHLSL compiler(spirv);

    // 获取资源
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    ShaderParameterMap map;
    ShaderParameterMapBuilder builder(map); 
    // 遍历 Uniform Buffers
    for (auto& ubo : resources.uniform_buffers)
    {
        
        // 获取 buffer 类型
        auto& type = compiler.get_type(ubo.base_type_id);
        // 获取buffer binding 和 set
        uint16_t binding = (uint16_t)compiler.get_decoration(ubo.id, spv::DecorationBinding);
        uint16_t set = (uint16_t)compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
        // 大小（字节）
        size_t size = compiler.get_declared_struct_size(type);
        builder.BeginUniformBuffer(ubo.name,size,set,binding);

        // 成员信息
        for (uint32_t i = 0; i < type.member_types.size(); ++i)
        {
            ShaderParameterMap::ParameterAllocation allocation;
            allocation.bufferIndex = builder.CurrentBufferIndex();
            std::string member_name = compiler.get_member_name(ubo.base_type_id, i);
            uint32_t offset = compiler.type_struct_member_offset(type, i);
            allocation.offset = (uint16_t)offset;
            allocation.type = ConvertBaseType(compiler.get_type(type.member_types[i]).basetype);
            allocation.vecSize = (uint8_t)compiler.get_type(type.member_types[i]).vecsize;
            allocation.columns = (uint8_t)compiler.get_type(type.member_types[i]).columns;
            builder.PushAllocation(member_name, allocation);
            auto& member_type = compiler.get_type(type.member_types[i]);
        }
        builder.EndUniformBuffer();
    }

    // 遍历纹理 / sampler
    for (auto& tex : resources.sampled_images)
    {
        uint16_t binding = (uint16_t)compiler.get_decoration(tex.id, spv::DecorationBinding);
        uint16_t set = (uint16_t)compiler.get_decoration(tex.id, spv::DecorationDescriptorSet);
        builder.PushSampler(tex.name,set,binding);
    }

    return map;
}
} // namespace Aether