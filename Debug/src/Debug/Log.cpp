#include "Log.h"
#include <iostream>

namespace Aether::Debug
{

Log& Log::Get()
{
	static Log log;
	return log;
}
Log::Log()
{
	spdlog::set_pattern("%^[%T] [%n] %v%$");
	m_Logger = spdlog::stdout_color_mt("Aether");
	m_Logger->set_level(spdlog::level::debug);
}
Log::~Log()
{
}
}