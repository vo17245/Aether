#pragma once
#include <vector>
#include "../../Core/Core.h"
namespace Aether
{
    using Buffer=std::vector<std::byte>;
    struct BufferView//Buffer[begin,end) 
    {
        BufferView(size_t _buffer,size_t _begin,size_t _end)
            :buffer(_buffer),begin(_begin),end(_end){}
        BufferView(const BufferView&)=default;
        BufferView(BufferView&&)=default;
        bool operator==(const BufferView& other)
        {
            return buffer==other.buffer&&begin==other.begin&&end==other.end;
        }
        BufferView operator=(const BufferView& other)
        {
            buffer=other.buffer;
            begin=other.begin;
            end=other.end;
        }
        size_t buffer;
        size_t begin;
        size_t end;
    };
}