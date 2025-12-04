#pragma once

#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <source_location>
#include <tuple>
#include <type_traits>
// Undefine Windows macros that include by spdlog
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
    template <typename... Args>
    static void Info(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
#ifdef AETHER_ENABLE_DEBUG_LOG
        Log::Get().GetLogger()->info(fmt, std::forward<Args>(args)...);
#endif
    }
    template <typename... Args>
    static void Debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
#ifdef AETHER_ENABLE_DEBUG_LOG
        Log::Get().GetLogger()->debug(fmt, std::forward<Args>(args)...);
#endif
    }
    template <typename... Args>
    static void Warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
#ifdef AETHER_ENABLE_DEBUG_LOG
        Log::Get().GetLogger()->warn(fmt, std::forward<Args>(args)...);
#endif
    }
    template <typename... Args>
    static void Error(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
#ifdef AETHER_ENABLE_DEBUG_LOG
        Log::Get().GetLogger()->error(fmt, std::forward<Args>(args)...);
#endif
    }
    template <typename... Args>
    static void Critical(spdlog::format_string_t<Args...> fmt, Args&&... args)
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
    inline const std::shared_ptr<spdlog::logger>& GetLogger() const
    {
        return m_Logger;
    }
};

namespace Detail
{
template <typename Tuple, size_t... I>
inline std::string FormatTuple(const Tuple& t, std::index_sequence<I...>)
{
    return std::vformat(std::get<0>(t), std::make_format_args(std::get<I + 1>(t)...));
}

} // namespace Detail
} // namespace Aether::Debug
namespace Aether
{
#ifdef AETHER_ENABLE_DEBUG_LOG
template <typename... Ts>
struct LogD
{
    LogD(Ts&&... ts, std::source_location loc = std::source_location::current())
    {
        auto tuple = std::forward_as_tuple(ts...);
        ::Aether::Debug::Log::Debug(
            "[{}:{}] {}", loc.file_name(), loc.line(),
            ::Aether::Debug::Detail::FormatTuple(tuple, std::make_index_sequence<sizeof...(Ts) - 1>{}));
    }
};

template <typename... Ts>
LogD(Ts&&... args) -> LogD<Ts...>;
template <typename... Ts>
struct LogI
{
    LogI(Ts&&... ts, std::source_location loc = std::source_location::current())
    {
        auto tuple = std::forward_as_tuple(ts...);
        ::Aether::Debug::Log::Info(
            "[{}:{}] {}", loc.file_name(), loc.line(),
            ::Aether::Debug::Detail::FormatTuple(tuple, std::make_index_sequence<sizeof...(Ts) - 1>{}));
    }
};
template <typename... Ts>
LogI(Ts&&... args) -> LogI<Ts...>;
template <typename... Ts>
struct LogW
{
    LogW(Ts&&... ts, std::source_location loc = std::source_location::current())
    {
        auto tuple = std::forward_as_tuple(ts...);
        ::Aether::Debug::Log::Warn(
            "[{}:{}] {}", loc.file_name(), loc.line(),
            ::Aether::Debug::Detail::FormatTuple(tuple, std::make_index_sequence<sizeof...(Ts) - 1>{}));
    }
};
template <typename... Ts>
LogW(Ts&&... args) -> LogW<Ts...>;
template <typename... Ts>
struct LogE
{
    LogE(Ts&&... ts, std::source_location loc = std::source_location::current())
    {
        auto tuple = std::forward_as_tuple(ts...);
        ::Aether::Debug::Log::Error(
            "[{}:{}] {}", loc.file_name(), loc.line(),
            ::Aether::Debug::Detail::FormatTuple(tuple, std::make_index_sequence<sizeof...(Ts) - 1>{}));
    }
};
template <typename... Ts>
LogE(Ts&&... args) -> LogE<Ts...>;
#else
template <typename... Ts>
struct LogD
{
};
template <typename... Ts>
struct LogI
{
};
template <typename... Ts>
struct LogW
{
};
template <typename... Ts>
struct LogE
{
};
#endif
} // namespace Aether