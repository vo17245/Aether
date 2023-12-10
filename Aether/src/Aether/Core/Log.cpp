#include "Log.h"
#include <iostream>
#include "Config.h"

namespace Aether
{

Log& Log::Get()
{
	static Log log;
	return log;
}
Log::Log()
{
	spdlog::set_pattern("%^[%T] [%n] %v%$");
	m_Logger = spdlog::stdout_color_mt(GetConfig().log_name);
	m_Logger->set_level(spdlog::level::debug);
}
Log::~Log()
{
}
}
