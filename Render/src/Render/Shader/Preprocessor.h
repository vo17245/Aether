#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <expected>
#include <span>
#include "Core/String.h"
namespace Aether
{
    namespace Shader
    {
        class Preprocessor
        {
        public:
            std::expected<std::string,std::string> Preprocess(const std::string_view code);
            inline void AddDefine(const std::string& key, const std::string& value)
            {
                m_Defines[key] = value;
            }
            inline void AddInclude(const std::string& include)
            {
                m_Includes.emplace_back(include);
            }
            // line end type,CRLF or LF
            enum class LineEnd
            {
                LF,
                CRLF
                // not support cr
            };
            static LineEnd DetectLineEnd(const std::string_view code);
        private:
            
            std::unordered_map<std::string, std::string> m_Defines;
            std::vector<std::string> m_Includes;
        };
    }
}