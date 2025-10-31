#pragma once
#include <Project/Project.h>

namespace Aether::Project
{
    class Texture:public Asset
    {
    public:

    private:
    };
    struct ImportTextureParams
    {
        std::string FilePath;
        std::string Address;
        std::string Name;
        bool SRGB=true;      
    };
    /**
     * @return error msg if failed,otherwise return nullopt
    */
    std::optional<std::string> ImportTexture(Project& project,ImportTextureParams& params);
}