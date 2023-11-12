#pragma once
#include "Log.h"
namespace Aether
{
	void AssertFunc(bool x, const char* s, const char* file, int line);
}
#ifdef AETHER_ENABLE_ASSERT
	#define AETHER_ASSERT(x) Aether::AssertFunc(bool(x),#x,__FILE__,__LINE__)
#else
	#define AETHER_ASSERT(x) ((void)0)
#endif