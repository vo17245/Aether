#include "Application.h"
#include <thread>
namespace Aether
{
InitParams Application::GetInitParams() const
{
    InitParams params;
    params.enableGlobalThreadPool = true;
    params.globalThreadPoolThreadCount = std::thread::hardware_concurrency();
    return params;
}
} // namespace Aether