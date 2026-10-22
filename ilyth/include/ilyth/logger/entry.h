#ifndef ILYTH_LOGGER_ENTRY_H
#define ILYTH_LOGGER_ENTRY_H

#include <ilyth/logger/log_level.h>
#include <ilyth/types.h>

#include <chrono>
#include <source_location>
#include <string_view>

namespace ilyth
{

/**
 * @brief One log record, handed to the formatter policy.
 *
 * Public on purpose: a policy reads its fields by name, which keeps the format
 * self-documenting and free of a positional contract (compare writing a bare
 * `std::format` call whose `{}` order becomes a frozen contract).
 *
 * @note Every string field is a non-owning view. They stay valid for the
 * duration of the enclosing log statement only.
 */
struct LogEntry
{
    LogLevel::Level level = LogLevel::Level::OFF;
    /// Logger that emitted the entry.
    std::string_view loggerName;
    /// Message from the caller.
    std::string_view message;
    std::chrono::system_clock::time_point timestamp;
    /// Elapsed time since first use.
    std::chrono::steady_clock::duration elapsed{};
    types::u64 threadID = 0;
    /// Reserved; always 0 until fibers exist.
    types::u64 fiberID = 0;
    std::string_view threadName;
    /// Call site of the log statement.
    std::source_location location{};
};

}  // namespace ilyth

#endif
