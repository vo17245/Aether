#include "Texture.h"
#include <Filesystem/Filesystem.h>
namespace Aether::Project
{
    void ImportTexture(Project& project,ImportTextureParams& params)
    {
        //copy image to address
        std::string destPath=project.GetContentDirPath()+params.Address;
        Filesystem::CopyFile(params.FilePath,destPath);
    }
}