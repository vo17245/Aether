#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <expected>
namespace Aether
{
class ShaderParameterMap
{
public:
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
        uint8_t vecSize;   // 1,2,3,4
        uint8_t columns;   // 1 for non-matrix, 2,3,4 for matrix
        uint16_t size;     // in bytes
    };
    bool FindParameterAllocation(const std::string& name, uint16_t& outBufferIndex, uint16_t& outBaseIndex,
                                 uint16_t& outSize) const;

private:
    
    std::unordered_map<std::string, ParameterAllocation> m_ParameterMap;
};
std::expected<ShaderParameterMap, std::string> ReflectShaderParameters(const std::vector<uint32_t>& spirv);
} // namespace Aether