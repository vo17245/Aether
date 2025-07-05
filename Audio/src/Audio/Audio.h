#pragma once
#include <string>
namespace Aether::Audio 
{
    /**
     * @return 0 on success, non-zero on failure
    */
    int Init();
    void Destory();
    /**
     * @param path utf-8 path
     * @return 0 on success, non-zero on failure
    */
    int Play(const char* path);
}