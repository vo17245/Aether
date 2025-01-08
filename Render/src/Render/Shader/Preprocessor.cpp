#include "Preprocessor.h"
#include <string>
#include "Filesystem/FilesystemApi.h"
#include "Filesystem/Utils.h"
#include "Lines.h"
#include "Filesystem/Filesystem.h"
#include <format>
namespace Aether {
namespace Shader {
Preprocessor::LineEnd Preprocessor::DetectLineEnd(const std::string_view code)
{
    
}
std::expected<std::string, std::string> Preprocessor::Preprocess(const std::string_view code)
{
    std::string result;
    auto lines = Lines(code);
    for (auto line : lines)
    {
        size_t pos = line.find("#include");
        if (pos != std::string::npos)
        {
            // handle #include
            // search for the path
            bool found = false;
            auto path = line.substr(pos + 8);
            for (auto include : m_Includes)
            {
                auto path2 = std::format("{}/{}", include, path);
                if (!Filesystem::Exists(path2))
                {
                    continue;
                }
                auto res = Filesystem::ReadFile(path2);
                if (!res)
                {
                    return std::unexpected(std::format("read file {} failed", path2));
                }
                result.insert(result.end(), res->begin(), res->end());
                result += "\n";
                found = true;
                break;
            }
            if (!found)
            {
                return std::unexpected(std::format("include file {} not found", path));
            }
        }
        else
        {
            // normal line
            result += line;
            result += "\n";
        }
    }
    return result;
}
}
} // namespace Aether::Shader