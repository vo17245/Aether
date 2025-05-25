#pragma once


#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
namespace Aether::Debug
{


class Log
{
public:
	template<typename... Args>
	static void Info(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
        #ifdef AETHER_ENABLE_DEBUG_LOG
		Log::Get().GetLogger()->info(fmt, std::forward<Args>(args)...);
        #endif
	}
	template<typename... Args>
	static void Debug(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
        #ifdef AETHER_ENABLE_DEBUG_LOG
		Log::Get().GetLogger()->debug(fmt, std::forward<Args>(args)...);
        #endif
	}
	template<typename... Args>
	static void Warn(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
        #ifdef AETHER_ENABLE_DEBUG_LOG
		Log::Get().GetLogger()->warn(fmt, std::forward<Args>(args)...);
        #endif
	}
	template<typename... Args>
	static void Error(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
        #ifdef AETHER_ENABLE_DEBUG_LOG
		Log::Get().GetLogger()->error(fmt, std::forward<Args>(args)...);
        #endif
	}
	template<typename... Args>
	static void Critical(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
        #ifdef AETHER_ENABLE_DEBUG_LOG
		Log::Get().GetLogger()->critical(fmt, std::forward<Args>(args)...);
        #endif
	}
private:
	Log();
	std::shared_ptr<spdlog::logger> m_Logger;
	~Log();
	static Log& Get();
	inline const std::shared_ptr<spdlog::logger>& GetLogger()const { return m_Logger; }
};

}