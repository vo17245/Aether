#pragma once
#include <string>
#ifdef _WIN32
#else
    #error "FileDialog is not implemented for this platform"
#endif
namespace Aether
{
    std::string OpenFileDialog();
}