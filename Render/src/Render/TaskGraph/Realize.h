#pragma once
#include <Core/Core.h>
namespace Aether::TaskGraph
{

template<typename Resource,typename Description>
Scope<Resource> Realize(const Description& desc)
{
    static_assert(sizeof(Resource)==0, "Missing realize implementation for description");
    return nullptr;
}
}
