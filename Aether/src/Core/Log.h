#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Core.h"
AETHER_NAMESPACE_BEGIN
class Log
{
public:
	~Log();
	static Log& Get();
	inline const std::shared_ptr<spdlog::logger>& GetLogger()const { return m_Logger; }
	
	template<typename... Args>
	static void Info(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->info(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Debug(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->debug(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Warn(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->warn(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Error(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->error(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Critical(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->critical(fmt, std::forward<Args>(args)...);
	}
private:
	Log();
	std::shared_ptr<spdlog::logger> m_Logger;
};
AETHER_NAMESPACE_END

#ifdef DEBUG
	#define debug_log(...) Aether::Log::Debug("{0}:{1}",__FILE__,__LINE__);Aether::Log::Get().GetLogger()->debug(__VA_ARGS__)
#else
	#define debug_log(...) ((void)0)
#endif




