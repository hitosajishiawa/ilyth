#ifndef ILYTH_LOGGER_LOG_H
#define ILYTH_LOGGER_LOG_H

#include <ilyth/logger/detail/buffer.h>
#include <ilyth/logger/detail/context.h>
#include <ilyth/logger/entry.h>
#include <ilyth/logger/logger.h>
#include <ilyth/utils/thread.h>

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <format>
#include <source_location>
#include <span>
#include <string_view>
#include <utility>

namespace ilyth::detail
{

inline constexpr auto LogMessageInlineCapacity = std::size_t{128};

/**
 * @brief Format the message, build the entry and submit it.
 *
 * The message goes through `std::format`, so a mismatch between the format
 * string and its arguments is diagnosed at the call site -- same guarantee as
 * calling `std::format` directly.
 *
 * @note Context is captured before rendering, and a rendering failure is
 * swallowed (best-effort logging), so a log statement never throws.
 */
template <class... Args>
void log_impl(Logger& logger,
              LogLevel::Level level,
              const std::source_location& location,
              std::format_string<Args...> fmt,
              Args&&... args) noexcept
{
    // Capture the call-site context before rendering, so the timestamp is
    // when the statement was issued rather than when formatting finished.
    const auto timestamp  = std::chrono::system_clock::now();
    const auto elapsed    = elapsedSinceLaunch();
    const auto threadID   = ilyth::currentThreadId();
    const auto fiberID    = currentFiberId();
    const auto threadName = ilyth::currentThreadName();

    LoggerBuffer<LogMessageInlineCapacity> message;
    try
    {
        formatTwoPass(message, [&](std::span<char> out) -> std::size_t {
            // Forward so `format_to_n` deduces the same parameter types `fmt`
            // was checked against. Safe to do twice: formatting never moves
            // its args.
            const auto result = std::format_to_n(
                out.data(), static_cast<std::ptrdiff_t>(out.size()), fmt,
                std::forward<Args>(args)...);
            return static_cast<std::size_t>(result.size);
        });
    }
    catch (...)
    {
        constexpr std::string_view note =
            "ilyth logger: message formatting failed\n";
        std::fwrite(note.data(), 1, note.size(), stderr);
        return;
    }

    const auto entry = LogEntry{
        .level      = level,
        .loggerName = logger.getName(),
        .message    = message.view(),
        .timestamp  = Timestamp{timestamp},
        .elapsed    = Elapsed{elapsed},
        .threadID   = threadID,
        .fiberID    = fiberID,
        .threadName = threadName,
        .location   = location,
    };
    logger.submit(entry);
}

}  // namespace ilyth::detail

/**
 * @brief Emit one record on @p logger .
 *
 * @p level is tested before anything is formatted, so a filtered-out line costs
 * only a pointer check.
 *
 * @note Why a macro and not a function template: C++ evaluates call arguments
 * eagerly, so a function would evaluate `__VA_ARGS__` even when the record is
 * filtered out. Here the filter is checked first, and the arguments are only
 * touched by @ref log_impl once the record passes.
 *
 * @warning Write arguments without side effects, unless you totally understand
 * that side effects in the arguments run only when the record passes the
 * filter, and you need to depend on this feature.
 */
#define ILYTH_LOG(logger, level, ...)                                         \
    do                                                                        \
    {                                                                         \
        const auto& ilyth_log_logger = (logger);                              \
        const auto ilyth_log_level   = (level);                               \
        if (ilyth_log_logger && ilyth_log_logger->shouldLog(ilyth_log_level)) \
            ::ilyth::detail::log_impl(*(ilyth_log_logger), ilyth_log_level,   \
                                      std::source_location::current(),        \
                                      __VA_ARGS__);                           \
    } while (0)

#define ILYTH_LOG_DEBUG(logger, ...) \
    ILYTH_LOG((logger), ::ilyth::LogLevel::Level::DEBUG, __VA_ARGS__)
#define ILYTH_LOG_INFO(logger, ...) \
    ILYTH_LOG((logger), ::ilyth::LogLevel::Level::INFO, __VA_ARGS__)
#define ILYTH_LOG_WARN(logger, ...) \
    ILYTH_LOG((logger), ::ilyth::LogLevel::Level::WARN, __VA_ARGS__)
#define ILYTH_LOG_ERROR(logger, ...) \
    ILYTH_LOG((logger), ::ilyth::LogLevel::Level::ERROR, __VA_ARGS__)
#define ILYTH_LOG_FATAL(logger, ...) \
    ILYTH_LOG((logger), ::ilyth::LogLevel::Level::FATAL, __VA_ARGS__)

#endif
