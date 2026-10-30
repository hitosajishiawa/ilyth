#ifndef ILYTH_LOGGER_LOGGER_H
#define ILYTH_LOGGER_LOGGER_H

#include <ilyth/export.h>
#include <ilyth/logger/appender.h>
#include <ilyth/logger/entry.h>
#include <ilyth/logger/formatter.h>
#include <ilyth/logger/level.h>

#include <memory>
#include <string>
#include <string_view>

namespace ilyth
{

namespace detail
{
class LoggerAccess;
inline constexpr auto FormattedEntryInlineCapacity = std::size_t{256};
}  // namespace detail

/**
 * @brief Named logger.
 *
 * Submissions format on the caller's own buffers and only the final write is
 * serialized at the appender, so multiple threads render concurrently. With no
 * appender of its own, a logger forwards everything to its parent root.
 */
class ILYTH_API Logger final
{
public:
    class Impl;
    explicit Logger(std::string name = "root");
    ~Logger();

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;

    [[nodiscard]] auto shouldLog(LogLevel::Level level) const noexcept -> bool;

    /**
     * @brief Set this logger's level filter.
     *
     * Applied before formatting: records less severe than @p level are dropped
     * by this logger and all of its appenders. An appender can narrow the set
     * further with @ref Appender::setFilter .
     *
     * @param level Lowest level this logger accepts. Defaults to @ref
     * LogLevel::Level::DEBUG ; pass @ref LogLevel::Level::OFF to silence it.
     */
    void setFilter(LogLevel::Level level) noexcept;

    /**
     * @brief Get this logger's level filter, i.e. the lowest accepted level.
     */
    [[nodiscard]] auto getFilter() const noexcept -> LogLevel::Level;

    [[nodiscard]] auto getName() const noexcept -> std::string_view;

    /**
     * @brief Install @p formatter (see @ref EntryFormatter).
     *
     * Pass the address of a free function you defined, or @ref defaultFormatter
     * to restore the default.
     */
    void setFormatter(EntryFormatter formatter);

    void addAppender(AppenderPtr appender);
    void removeAppender(const AppenderPtr& appender);
    void clearAppenders();

    void flush();
    void sync();

    /**
     * @brief Low-level entry: submit an already built entry.
     *
     * Prefer the `ILYTH_LOG_*` macros. @p entry's views must stay valid until
     * this returns.
     */
    void submit(const LogEntry& entry) noexcept;

private:
    std::unique_ptr<Impl> mImpl;

    friend class detail::LoggerAccess;
};

using LoggerPtr = std::shared_ptr<Logger>;

[[nodiscard]] ILYTH_API auto GetRootLogger() -> LoggerPtr;
[[nodiscard]] ILYTH_API auto GetLogger(std::string_view name) -> LoggerPtr;

}  // namespace ilyth

#endif
