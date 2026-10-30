#ifndef ILYTH_SRC_LOGGER_APPENDER_IMPL_H
#define ILYTH_SRC_LOGGER_APPENDER_IMPL_H

#include <ilyth/logger/appender.h>

#include <atomic>
#include <mutex>
#include <string_view>

namespace ilyth
{

class Appender::Impl
{
public:
    virtual ~Impl() = default;

    void append(LogLevel::Level level, std::string_view record) noexcept;
    void setFilter(LogLevel::Level level) noexcept;
    [[nodiscard]] auto getFilter() const noexcept -> LogLevel::Level;

    void flush();
    void sync();

protected:
    virtual void writeUnlocked(std::string_view record) noexcept = 0;
    virtual void flushUnlocked() noexcept                        = 0;
    virtual void syncUnlocked() noexcept                         = 0;

    std::mutex mMutex;

private:
    std::atomic<LogLevel::Level> mFilter{LogLevel::Level::DEBUG};
};

namespace detail
{

/// Bridges to `Appender`'s private constructor and implementation.
class AppenderAccess final
{
    AppenderAccess()                      = delete;
    AppenderAccess(const AppenderAccess&) = delete;
    AppenderAccess(AppenderAccess&&)      = delete;

public:
    [[nodiscard]] static auto MakeStdoutAppender() -> AppenderPtr;
    [[nodiscard]] static auto MakeFileAppender(std::string fileName)
        -> AppenderPtr;

    static void Append(const AppenderPtr& appender,
                       LogLevel::Level level,
                       std::string_view record) noexcept;
};

}  // namespace detail

}  // namespace ilyth

#endif
