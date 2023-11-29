#pragma once


#include "Core.h"
#include <sstream>
#include <thread>
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

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

template<typename... Args>
inline void DebugLogFunc(const char* file, int line, const char* fmt, Args&&... args)
{
	static_assert(sizeof(std::thread::id)==sizeof(unsigned int));
	auto msg = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
	std::thread::id thread_id=std::this_thread::get_id();
	unsigned int uint_thread_id=*((unsigned int*)(&thread_id));
	std::string debugMsg = fmt::format("[thread {}][{}:{}] {}",uint_thread_id, file, line, msg);
	Log::Debug("{}",debugMsg.c_str());
}
template<typename... Args>
inline void DebugLogErrorFunc(const char* file, int line, const char* fmt, Args&&... args)
{
	static_assert(sizeof(std::thread::id)==sizeof(unsigned int));
	auto msg = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
	std::thread::id thread_id=std::this_thread::get_id();
	unsigned int uint_thread_id=*((unsigned int*)(&thread_id));
	std::string debugMsg = fmt::format("[thread {}][{}:{}] {}", uint_thread_id,file, line, msg);
	Log::Error("{}", debugMsg.c_str());
}
AETHER_NAMESPACE_END


#ifdef AETHER_ENABLE_DEBUG_LOG
	#define AETHER_DEBUG_LOG(...) Aether::DebugLogFunc(__FILE__,__LINE__,__VA_ARGS__)
	#define AETHER_DEBUG_LOG_ERROR(...) Aether::DebugLogErrorFunc(__FILE__,__LINE__,__VA_ARGS__)
#else
	#define AETHER_DEBUG_LOG(...) ((void)0)
	#define AETHER_DEBUG_LOG_ERROR(...) ((void)0)
#endif



