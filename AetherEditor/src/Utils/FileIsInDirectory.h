#pragma once
#include <string>
#include <Filesystem/Filesystem.h>
namespace Utils
{
    inline bool FileIsInDirectory(const std::string& filePath, const std::string& directoryPath)
    {
        ::Aether::Filesystem::Path fp(filePath);
        ::Aether::Filesystem::Path dp(directoryPath);
        if(fp.Parent()==dp)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}