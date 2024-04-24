#pragma once
#include <string>
#include <variant>
namespace Aether
{
    namespace AudioServerCommand
    {
        struct PlayMp3FileCommand
        {
            std::string path;
            PlayMp3FileCommand(const std::string& _path):path(_path){}
            PlayMp3FileCommand(const PlayMp3FileCommand&)=default;
        };
        using Command=std::variant<std::monostate,PlayMp3FileCommand>;
    }
}