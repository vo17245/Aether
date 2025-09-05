#pragma once
#include <Render/Render.h>
namespace Aether
{
    class ShaderParameterMap
    {
    public:
        bool FindParameterAllocation(const std::string& name,uint16_t& outBufferIndex,uint16_t& outBaseIndex,uint16_t& outSize)const;
    private:
        struct ParameterAllocation
        {
            uint16_t bufferIndex;
            uint16_t baseIndex;
            uint16_t size;
        };
        std::unordered_map<std::string,ParameterAllocation> m_ParameterMap;
    };
}