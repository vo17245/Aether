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
        std::string Tag;
        bool SRGB=true;      
    };
    void ImportTexture(Project& project,ImportTextureParams& params);
}