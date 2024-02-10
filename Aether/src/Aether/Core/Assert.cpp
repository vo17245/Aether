#include "Assert.h"
namespace Aether
{
	void AssertFunc(bool x, const char* s, const char* file, int line)
	{
		if (!(x))
		{
			Log::Error("[{}:{}] [Assert Failed] {}", file, line, s);
			std::abort();
		}
		
	}
}
