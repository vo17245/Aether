#pragma once
#include "Core/Base.h"
#include <vector>
#include <span>
#include <stdio.h>
namespace Aether {
namespace Filesystem {

// clang-format off
/**
* @note FileHandle 是一个非透明的句柄
*                  在Windows下为HANDLE (aka void*)
*                  在Linux下为FILE* (aka void*)
*/
struct FileHandle
{
    void* data;
};
/**
* @note Entry 是一个非透明的数据
*                  在Windows下为WIN32_FIND_DAT
*/

// clang-format on
enum class FileType : uint32_t
{
    Regular,
    Directory,
};
enum class Action : uint32_t
{
    Create = Bit(0),
    Remove = Bit(1),
    Rename = Bit(2),
    Append = Bit(3),
    Read = Bit(4),
    Overwrite = Bit(5),
};
using ActionFlags = uint32_t;
inline ActionFlags operator|(Action lhs, Action rhs)
{
    return static_cast<ActionFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}
inline ActionFlags operator&(Action lhs, Action rhs)
{
    return static_cast<ActionFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}
inline ActionFlags operator~(Action action)
{
    return static_cast<ActionFlags>(~static_cast<uint32_t>(action));
}
inline ActionFlags operator&(ActionFlags lhs, Action rhs)
{
    return lhs & static_cast<ActionFlags>(rhs);
}

}
} // namespace Aether::Filesystem