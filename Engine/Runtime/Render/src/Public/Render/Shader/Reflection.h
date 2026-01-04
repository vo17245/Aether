#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <expected>
namespace Aether
{
class ShaderParameterMapBuilder;
class ShaderParameterMap
{
public:
	friend class ShaderParameterMapBuilder;
    enum class BaseType
    {
        Unknown,
        Void,
        Boolean,
        SByte,
        UByte,
        Short,
        UShort,
        Int,
        UInt,
        Int64,
        UInt64,
        AtomicCounter,
        Half,
        Float,
        Double,
        Struct,
        Image,
        SampledImage,
        Sampler,
        AccelerationStructure,
        RayQuery,
        CoopVecNV,
    };
    struct ParameterAllocation
    {
        uint16_t bufferIndex;
        uint16_t offset;
        BaseType type;
        uint8_t vecSize; // 1,2,3,4
        uint8_t columns; // 1 for non-matrix, 2,3,4 for matrix
    };
    struct BufferInfo
    {
		std::string name;
		uint16_t size;// struct size in bytes
		uint16_t set;
		uint16_t binding;
        uint16_t allocationBegin = 0; // allocation begin
        uint16_t allocationCount = 0;  // allocation count
    };
	struct SamplerInfo
	{
		std::string name;
	};
	
    ParameterAllocation* FindParameterAllocation(const std::string& name)
    {
        auto it = m_ParameterMap.find(name);
        if (it == m_ParameterMap.end())
            return nullptr;
        return &m_Allocations[it->second];
    }
    inline std::vector<BufferInfo>& GetUniformBuffers()
    {
        return m_UniformBuffers;
    }
    inline std::vector<ParameterAllocation>& GetAllocations()
    {
        return m_Allocations;
    }

private:
    std::vector<ParameterAllocation> m_Allocations;
    std::unordered_map<std::string, uint16_t> m_ParameterMap;
    std::vector<BufferInfo> m_UniformBuffers;
	std::vector<BufferInfo> m_StorageBuffers;
	std::vector<SamplerInfo> m_Samplers;
};

std::expected<ShaderParameterMap, std::string> ReflectShaderParameters(const std::vector<uint32_t>& spirv);
} // namespace Aether