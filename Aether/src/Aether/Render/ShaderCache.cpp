#include "ShaderCache.h"
#include "Aether/Core/Config.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/ShaderSource.h"
#include <stdint.h>

namespace Aether
{
    Ref<Shader> ShaderCache::GetShader(BuiltinShaderSignature signature)
    {
        if(signature.type==BuiltinShaderType::BASIC)
        {
            // return cached shader
            if(m_ShaderCache.find(signature.value)!=m_ShaderCache.end())
            {
                return m_ShaderCache[signature.value];
            }
            //create shader
            if(signature.macro!=0)
            {
                AETHER_DEBUG_LOG_WARN("ignore macro for BASIC shader");
            }
            if(m_SourceCache.find(signature.value)==m_SourceCache.end())
            {
                auto opt_src=ShaderSource::LoadBuiltin(signature.type);
                AETHER_ASSERT(opt_src&&"failed to load builtin shader");
                m_SourceCache[(uint32_t)signature.type]=opt_src.value();
            }
            auto shader=Shader::Create(*m_SourceCache[(uint32_t)signature.type]);
            AETHER_ASSERT(shader&&"failed to create shader");
            m_ShaderCache[signature.value]=shader;
            return shader;
        }
        else
        {
            AETHER_ASSERT(false&&"BuiltinShaderType not supported");
            AETHER_DEBUG_LOG_ERROR("BuiltinShaderType not supported");
            return nullptr;
        }
    }
}