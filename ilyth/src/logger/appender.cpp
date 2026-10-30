#include "logger/appender_impl.h"

#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#if ILYTH_PLATFORM_WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace ilyth
{

Appender::Appender(std::unique_ptr<Impl> impl) noexcept : mImpl(std::move(impl))
{}

Appender::~Appender() = default;

void Appender::setFilter(LogLevel::Level level) noexcept
{
    mImpl->setFilter(level);
}

auto Appender::getFilter() const noexcept -> LogLevel::Level
{
    return mImpl->getFilter();
}

void Appender::flush()
{
    mImpl->flush();
}

void Appender::sync()
{
    mImpl->sync();
}

void Appender::Impl::append(LogLevel::Level level,
                            std::string_view record) noexcept
{
    if (static_cast<types::u8>(level) < static_cast<types::u8>(getFilter()))
    {
        return;
    }
    // std::mutex::lock() may throw; keep it from escaping the logging path.
    try
    {
        std::lock_guard<std::mutex> lock(mMutex);
        writeUnlocked(record);
        if (level == LogLevel::Level::FATAL)
        {
            flushUnlocked();
        }
    }
    catch (...)
    {
        constexpr std::string_view message =
            "ilyth logger: appender operation failed\n";
        std::fwrite(message.data(), 1, message.size(), stderr);
    }
}

void Appender::Impl::setFilter(LogLevel::Level level) noexcept
{
    mFilter.store(level, std::memory_order_release);
}

auto Appender::Impl::getFilter() const noexcept -> LogLevel::Level
{
    return mFilter.load(std::memory_order_acquire);
}

void Appender::Impl::flush()
{
    std::lock_guard<std::mutex> lock(mMutex);
    flushUnlocked();
}

void Appender::Impl::sync()
{
    std::lock_guard<std::mutex> lock(mMutex);
    syncUnlocked();
}

namespace
{

class StdoutAppenderImpl final : public Appender::Impl
{
protected:
    void writeUnlocked(std::string_view record) noexcept override
    {
        // One bulk write per record; stdio adds its own buffering
        // (line-buffered on a tty, fully buffered otherwise).
        std::fwrite(record.data(), 1, record.size(), stdout);
    }

    void flushUnlocked() noexcept override { std::fflush(stdout); }

    void syncUnlocked() noexcept override { flushUnlocked(); }
};

class FileAppenderImpl final : public Appender::Impl
{
public:
    explicit FileAppenderImpl(const std::string& fileName)
    {
        if (fileName.empty())
        {
            throw std::invalid_argument("logger file name cannot be empty");
        }
        mFile = std::fopen(fileName.c_str(), "ab");
        if (mFile == nullptr)
        {
            throw std::system_error(errno, std::generic_category(),
                                    "failed to open logger file: " + fileName);
        }
    }

    ~FileAppenderImpl() override
    {
        if (mFile != nullptr)
        {
            std::fflush(mFile);
            std::fclose(mFile);
        }
    }

protected:
    void writeUnlocked(std::string_view record) noexcept override
    {
        std::fwrite(record.data(), 1, record.size(), mFile);
    }

    void flushUnlocked() noexcept override { std::fflush(mFile); }

    void syncUnlocked() noexcept override
    {
        std::fflush(mFile);
#if ILYTH_PLATFORM_WIN32
        ::_commit(::_fileno(mFile));
#else
        ::fdatasync(::fileno(mFile));
#endif
    }

private:
    std::FILE* mFile = nullptr;
};

}  // namespace

auto detail::AppenderAccess::MakeStdoutAppender() -> AppenderPtr
{
    return AppenderPtr(new Appender(std::make_unique<StdoutAppenderImpl>()));
}

auto detail::AppenderAccess::MakeFileAppender(std::string fileName)
    -> AppenderPtr
{
    return AppenderPtr(
        new Appender(std::make_unique<FileAppenderImpl>(fileName)));
}

void detail::AppenderAccess::Append(const AppenderPtr& appender,
                                    LogLevel::Level level,
                                    std::string_view record) noexcept
{
    if (appender != nullptr)
    {
        appender->mImpl->append(level, record);
    }
}

auto MakeStdoutAppender() -> AppenderPtr
{
    return detail::AppenderAccess::MakeStdoutAppender();
}

auto MakeFileAppender(std::string fileName) -> AppenderPtr
{
    return detail::AppenderAccess::MakeFileAppender(std::move(fileName));
}

}  // namespace ilyth
