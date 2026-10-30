#ifndef ILYTH_SRC_LOGGER_LOGGER_IMPL_H
#define ILYTH_SRC_LOGGER_LOGGER_IMPL_H

#include <ilyth/logger/entry.h>
#include <ilyth/logger/formatter.h>
#include <ilyth/logger/logger.h>

#include <atomic>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

namespace ilyth
{

class Logger::Impl
{
public:
    explicit Impl(std::string name);

    [[nodiscard]] auto shouldLog(LogLevel::Level level) const noexcept -> bool;
    void submit(const LogEntry& entry) noexcept;

    void setFilter(LogLevel::Level level) noexcept;
    [[nodiscard]] auto getFilter() const noexcept -> LogLevel::Level;
    [[nodiscard]] auto getName() const noexcept -> std::string_view;

    void setFormatter(EntryFormatter formatter);
    void addAppender(AppenderPtr appender);
    void removeAppender(const AppenderPtr& appender);
    void clearAppenders();

    void flush();
    void sync();

    void setRoot(const LoggerPtr& root) noexcept;

private:
    std::string mName;
    std::atomic<LogLevel::Level> mFilter{LogLevel::Level::DEBUG};

    // Shared: submissions render concurrently and only touch this to read the
    // formatter and appender list. Writers are rare (setup time).
    mutable std::shared_mutex mMutex;
    EntryFormatter mFormatter = &defaultVerboseFormatter;
    std::vector<AppenderPtr> mAppenders;

    LoggerPtr mRoot;
};

namespace detail
{

/// Bridges to `Logger`'s private implementation for the logger registry.
class LoggerAccess final
{
    LoggerAccess()                    = delete;
    LoggerAccess(const LoggerAccess&) = delete;
    LoggerAccess(LoggerAccess&&)      = delete;

public:
    static void SetRoot(Logger& logger, const LoggerPtr& root) noexcept;
};

}  // namespace detail

}  // namespace ilyth

#endif
