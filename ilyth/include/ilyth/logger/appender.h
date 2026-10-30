#ifndef ILYTH_LOGGER_APPENDER_H
#define ILYTH_LOGGER_APPENDER_H

#include <ilyth/export.h>
#include <ilyth/logger/level.h>

#include <memory>
#include <string>

namespace ilyth
{

namespace detail
{
class AppenderAccess;
}

/**
 * @brief Output sink. Synchronous: one `fwrite` per record, serialized by an
 * internal `std::mutex`.
 *
 * The mutex is held only around the write -- formatting happens before it, on
 * the caller's own buffers -- so the lock is never held across the expensive
 * part.
 */
class ILYTH_API Appender final
{
public:
    class Impl;

    ~Appender();
    Appender(const Appender&)            = delete;
    Appender& operator=(const Appender&) = delete;
    Appender(Appender&&)                 = delete;
    Appender& operator=(Appender&&)      = delete;

    /**
     * @brief Set this sink's level filter.
     *
     * Records less severe than @p level are dropped by this appender only;
     * other appenders are unaffected. See @ref LogLevel::Level for the
     * ordering.
     *
     * @param level Lowest level this appender accepts. Defaults to @ref
     * LogLevel::Level::DEBUG , so nothing is filtered; pass @ref
     * LogLevel::Level::OFF to silence this appender.
     */
    void setFilter(LogLevel::Level level) noexcept;

    /**
     * @brief Get this sink's level filter, i.e. the lowest accepted level.
     */
    [[nodiscard]] auto getFilter() const noexcept -> LogLevel::Level;

    /**
     * Push library buffers down to the OS (`fflush`).
     */
    void flush();
    /**
     * Push OS buffers down to the device (`fdatasync`); no-op for stdout.
     */
    void sync();

private:
    explicit Appender(std::unique_ptr<Impl> impl) noexcept;
    std::unique_ptr<Impl> mImpl;

    friend class detail::AppenderAccess;
};

using AppenderPtr = std::shared_ptr<Appender>;

[[nodiscard]] ILYTH_API auto MakeStdoutAppender() -> AppenderPtr;
[[nodiscard]] ILYTH_API auto MakeFileAppender(std::string fileName)
    -> AppenderPtr;

}  // namespace ilyth

#endif
